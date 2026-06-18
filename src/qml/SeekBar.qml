import QtQuick

Item {
    id: seekRoot
    height: 30

    property real position: 0      // In ms
    property real duration: 0      // In ms
    property bool canSeek: true

    signal seekRequested(real positionMs)

    readonly property bool dragging: mouseArea.pressed
    property real dragPosition: 0  // In ms

    readonly property real displayPosition: dragging ? dragPosition : position
    readonly property real progress: duration > 0 ? Math.min(1.0, Math.max(0.0, displayPosition / duration)) : 0.0

    function formatTime(ms) {
        if (isNaN(ms) || ms < 0) ms = 0;
        var totalSeconds = Math.floor(ms / 1000);
        var seconds = totalSeconds % 60;
        var minutes = Math.floor(totalSeconds / 60) % 60;
        var hours = Math.floor(totalSeconds / 3600);

        var secStr = seconds < 10 ? "0" + seconds : seconds;
        if (hours > 0) {
            var minStr = minutes < 10 ? "0" + minutes : minutes;
            return hours + ":" + minStr + ":" + secStr;
        } else {
            return minutes + ":" + secStr;
        }
    }

    Row {
        id: timeRow
        anchors.fill: parent
        spacing: 8

        Text {
            id: currentText
            text: seekRoot.formatTime(seekRoot.displayPosition)
            color: "#b0b0c0"
            font.pixelSize: 11
            font.family: "Outfit"
            width: 40
            horizontalAlignment: Text.AlignRight
            anchors.verticalCenter: parent.verticalCenter
        }

        // Track bar
        Item {
            id: trackContainer
            width: parent.width - 96
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter
            
            Rectangle {
                id: trackBg
                width: parent.width
                height: 4
                radius: 2
                color: "#22ffffff"
                anchors.centerIn: parent
            }

            Rectangle {
                id: trackHighlight
                width: parent.width * seekRoot.progress
                height: 4
                radius: 2
                color: "#ff007f" // vibrant neon pink
                anchors.left: trackBg.left
                anchors.verticalCenter: trackBg.verticalCenter

                // Neon glow
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: "#7f00ff" } // violet
                    GradientStop { position: 1.0; color: "#ff007f" } // pink
                }
            }

            // Glow handle
            Rectangle {
                id: handle
                width: mouseArea.containsMouse || seekRoot.dragging ? 12 : 8
                height: width
                radius: width / 2
                color: "#ffffff"
                anchors.horizontalCenter: trackBg.left
                anchors.horizontalCenterOffset: trackBg.width * seekRoot.progress
                anchors.verticalCenter: trackBg.verticalCenter

                border.width: 1.5
                border.color: "#ff007f"

                Behavior on width {
                    NumberAnimation { duration: 150 }
                }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: seekRoot.canSeek
                enabled: seekRoot.canSeek
                cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor

                onPressed: function(mouse) { updateDragPosition(mouse) }
                onPositionChanged: function(mouse) {
                    if (pressed) {
                        updateDragPosition(mouse);
                    }
                }
                onReleased: {
                    seekRoot.seekRequested(seekRoot.dragPosition);
                }

                function updateDragPosition(mouseEvent) {
                    var percentage = Math.min(1.0, Math.max(0.0, mouseEvent.x / width));
                    seekRoot.dragPosition = percentage * seekRoot.duration;
                }
            }
        }

        Text {
            id: durationText
            text: seekRoot.formatTime(seekRoot.duration)
            color: "#b0b0c0"
            font.pixelSize: 11
            font.family: "Outfit"
            width: 40
            horizontalAlignment: Text.AlignLeft
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
