import QtQuick
import QtQuick.Controls

Window {
    id: mainWindow
    visible: true
    width: settings.windowWidth > 0 ? settings.windowWidth : 400
    height: settings.windowHeight > 0 ? settings.windowHeight : 560
    
    // Set position if saved
    x: settings.windowX >= 0 ? settings.windowX : (Screen.width - width) / 2
    y: settings.windowY >= 0 ? settings.windowY : (Screen.height - height) / 2

    color: "transparent"
    opacity: settings.windowOpacity
    flags: Qt.Window | Qt.FramelessWindowHint | (settings.alwaysOnTop ? Qt.WindowStaysOnTopHint : 0)

    title: "Groovr"

    // Coalesce window movement/resize settings writes to avoid disk flood
    Timer {
        id: saveSettingsTimer
        interval: 1000
        repeat: false
        onTriggered: {
            settings.windowX = mainWindow.x;
            settings.windowY = mainWindow.y;
            settings.windowWidth = mainWindow.width;
            settings.windowHeight = mainWindow.height;
        }
    }

    onXChanged: saveSettingsTimer.restart()
    onYChanged: saveSettingsTimer.restart()
    onWidthChanged: saveSettingsTimer.restart()
    onHeightChanged: saveSettingsTimer.restart()

    // Outer Glow Border and Translucent Background Panel
    GlassPanel {
        id: mainPanel
        anchors.fill: parent
        anchors.margins: 10 // Leave margin for shadow glow

        property bool settingsOpen: false

        // Close settings when clicking outside the popover
        TapHandler {
            onTapped: function(eventPoint) {
                if (mainPanel.settingsOpen && !settingsPopover.contains(settingsPopover.mapFromItem(mainPanel, eventPoint.position)))
                    mainPanel.settingsOpen = false
            }
        }

        // Native Window Dragging Handler (Active on background only)
        DragHandler {
            target: null
            grabPermissions: PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.CanTakeOverFromHandlersOfSameType
            onActiveChanged: {
                if (active) {
                    mainWindow.startSystemMove();
                }
            }
        }

        // Header Actions
        Row {
            id: header
            anchors.top: parent.top
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.right: parent.right
            anchors.rightMargin: 16
            height: 30
            spacing: 8
            z: 100 // Ensure dropdown overlays have priority

            PlayerSelector {
                id: playerPicker
                width: 140
                height: 30
                activePlayerName: mediaController.activePlayerName
                activePlayerService: mediaController.activePlayerService
                preferredPlayerService: settings.preferredPlayer
                availableServices: mediaController.availablePlayers
                availableIdentities: mediaController.availablePlayerIdentities
                
                onPlayerSelected: function(serviceName) {
                    mediaController.selectPlayer(serviceName);
                }
            }

            Item {
                // Spacer
                width: parent.width - playerPicker.width - adMuterButton.width - settingsButton.width - pinButton.width - closeButton.width - 40
                height: 1
            }

            // Ad Muter Toggle
            Rectangle {
                id: adMuterButton
                width: 30
                height: 30
                radius: 6
                color: adMuterMouse.pressed ? "#20ffffff" : (adMuterMouse.containsMouse ? "#12ffffff" : "transparent")
                border.color: adMuter.enabled ? (adMuter.isAd ? "#ff6600" : "#00ffff") : "transparent"
                border.width: 1

                Text {
                    text: "🔇"
                    font.pixelSize: 13
                    anchors.centerIn: parent
                    opacity: adMuter.enabled ? 1.0 : 0.35
                    color: adMuter.isAd ? "#ff6600" : "#ffffff"
                }

                MouseArea {
                    id: adMuterMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: adMuter.enabled = !adMuter.enabled
                }

                ToolTip.visible: adMuterMouse.containsMouse
                ToolTip.text: adMuter.enabled ? (adMuter.isAd ? "Muting ad" : "Ad muter on") : "Ad muter off"
                ToolTip.delay: 600
            }

            // Settings Gear
            Rectangle {
                id: settingsButton
                width: 30
                height: 30
                radius: 6
                color: settingsMouse.pressed ? "#20ffffff" : (settingsMouse.containsMouse ? "#12ffffff" : "transparent")
                border.color: mainPanel.settingsOpen ? "#00ffff" : "transparent"
                border.width: 1

                Text {
                    text: "⚙"
                    font.pixelSize: 14
                    anchors.centerIn: parent
                    color: mainPanel.settingsOpen ? "#00ffff" : "#a0a0c0"
                }

                MouseArea {
                    id: settingsMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: mainPanel.settingsOpen = !mainPanel.settingsOpen
                }
				ToolTip.visible: settingsMouse.containsMouse
                ToolTip.text: "Settings"
                ToolTip.delay: 600
            }

            // Always On Top Pin
            Rectangle {
                id: pinButton
                width: 30
                height: 30
                radius: 6
                color: pinMouse.pressed ? "#20ffffff" : (pinMouse.containsMouse ? "#12ffffff" : "transparent")
                border.color: settings.alwaysOnTop ? "#00ffff" : "transparent"
                border.width: 1

                Text {
                    text: "📌"
                    font.pixelSize: 12
                    anchors.centerIn: parent
                    opacity: settings.alwaysOnTop ? 1.0 : 0.4
                }

                MouseArea {
                    id: pinMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: settings.alwaysOnTop = !settings.alwaysOnTop
                }
				ToolTip.visible: pinMouse.containsMouse
                ToolTip.text: "Always on top"
                ToolTip.delay: 600
            }

            // Close Button
            Rectangle {
                id: closeButton
                width: 30
                height: 30
                radius: 6
                color: closeMouse.pressed ? "#40ff0055" : (closeMouse.containsMouse ? "#20ff0055" : "transparent")

                Text {
                    text: "✕"
                    color: closeMouse.containsMouse ? "#ff4080" : "#a0a0c0"
                    font.pixelSize: 12
                    font.bold: true
                    anchors.centerIn: parent
                }

                MouseArea {
                    id: closeMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: mainWindow.close()
                }
            }
        }

        // Center Content Layout
        Column {
            id: contentColumn
            anchors.top: header.bottom
            anchors.topMargin: 12
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: footer.top
            anchors.bottomMargin: 12
            spacing: 14

            // The Groovr Visualizer
            GroovrRecord {
                id: visualizer
                width: Math.min(parent.width - 40, parent.height - 110)
                height: width
                anchors.horizontalCenter: parent.horizontalCenter
                
                coverUrl: mediaController.coverArtUrl
                isPlaying: mediaController.isPlaying
                progress: mediaController.durationMs > 0 ? (mediaController.positionMs / mediaController.durationMs) : 0.0
            }

            // Metadata Texts
            Column {
                width: parent.width - 40
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 4

                Text {
                    text: mediaController.trackTitle
                    color: "#ffffff"
                    font.pixelSize: 16
                    font.bold: true
                    font.family: "Outfit"
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    width: parent.width
                }

                Text {
                    text: mediaController.trackArtist
                    color: "#b0b0c8"
                    font.pixelSize: 12
                    font.family: "Outfit"
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    width: parent.width
                }

                Text {
                    text: mediaController.trackAlbum
                    color: "#6c6c84"
                    font.pixelSize: 10
                    font.family: "Outfit"
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    width: parent.width
                    visible: text.length > 0
                }
            }
        }

        // Settings Popover
        Rectangle {
            id: settingsPopover
            visible: mainPanel.settingsOpen
            width: 210
            height: 76
            z: 200
            anchors.top: header.bottom
            anchors.topMargin: 4
            anchors.right: parent.right
            anchors.rightMargin: 0
            radius: 10
            color: "#e00f0f1d"
            border.color: "#28ffffff"
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Text {
                    text: "Window Opacity"
                    color: "#a0a0c0"
                    font.pixelSize: 11
                }

                Row {
                    width: parent.width
                    spacing: 8

                    Item {
                        id: opacitySliderTrack
                        width: parent.width - opacityLabel.width - 8
                        height: 20

                        Rectangle {
                            width: parent.width
                            height: 4
                            radius: 2
                            anchors.verticalCenter: parent.verticalCenter
                            color: "#30ffffff"

                            Rectangle {
                                width: parent.width * settings.windowOpacity
                                height: parent.height
                                radius: parent.radius
                                color: "#00ffff"
                            }
                        }

                        Rectangle {
                            width: 14
                            height: 14
                            radius: 7
                            color: "#00ffff"
                            border.color: "#80ffffff"
                            border.width: 1
                            anchors.verticalCenter: parent.verticalCenter
                            x: settings.windowOpacity * (opacitySliderTrack.width - width)
                        }

                        MouseArea {
                            anchors.fill: parent
                            onPressed: function(mouse) {
                                settings.windowOpacity = Math.max(0.15, Math.min(1.0, mouse.x / (opacitySliderTrack.width - 14)));
                            }
                            onPositionChanged: function(mouse) {
                                if (pressed)
                                    settings.windowOpacity = Math.max(0.15, Math.min(1.0, mouse.x / (opacitySliderTrack.width - 14)));
                            }
                        }
                    }

                    Text {
                        id: opacityLabel
                        text: Math.round(settings.windowOpacity * 100) + "%"
                        color: "#ffffff"
                        font.pixelSize: 11
                        width: 30
                        horizontalAlignment: Text.AlignRight
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        // Footer Controls
        Column {
            id: footer
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.right: parent.right
            anchors.rightMargin: 20
            spacing: 6

            SeekBar {
                id: progressSeek
                width: parent.width
                position: mediaController.positionMs
                duration: mediaController.durationMs
                canSeek: mediaController.canSeek
                onSeekRequested: function(posMs) {
                    mediaController.seek(posMs);
                }
            }

            ControlBar {
                id: controls
                width: parent.width
                isPlaying: mediaController.isPlaying
                canGoNext: mediaController.canGoNext
                canGoPrevious: mediaController.canGoPrevious
                canPlay: mediaController.canPlay
                canPause: mediaController.canPause

                onPrevClicked: mediaController.previous()
                onPlayPauseClicked: mediaController.playPause()
                onNextClicked: mediaController.next()
            }
        }
    }
}
