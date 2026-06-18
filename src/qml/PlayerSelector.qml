import QtQuick
import QtQuick.Controls

Item {
    id: selectorRoot
    width: 150
    height: 30

    property string activePlayerName: "No Player"
    property string activePlayerService: ""
    property string preferredPlayerService: ""
    property var availableServices: []
    property var availableIdentities: []

    signal playerSelected(string serviceName)

    property bool isOpen: false

    // Active Player Display Button
    Rectangle {
        id: button
        anchors.fill: parent
        color: buttonMouse.pressed ? "#20ffffff" : (buttonMouse.containsMouse ? "#12ffffff" : "#08ffffff")
        border.color: selectorRoot.isOpen ? "#00ffff" : "#1affffff"
        border.width: 1
        radius: 6

        Behavior on color { ColorAnimation { duration: 150 } }
        Behavior on border.color { ColorAnimation { duration: 150 } }

        Row {
            anchors.centerIn: parent
            spacing: 6
            width: parent.width - 16

            Text {
                text: "🎵"
                font.pixelSize: 11
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: selectorRoot.activePlayerName
                color: "#ffffff"
                font.pixelSize: 11
                font.family: "Outfit"
                elide: Text.ElideRight
                width: parent.width - 24
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: selectorRoot.isOpen ? "▴" : "▾"
                color: "#80ffffff"
                font.pixelSize: 10
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        MouseArea {
            id: buttonMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: selectorRoot.isOpen = !selectorRoot.isOpen
        }
    }

    // Dismiss overlay to capture clicks outside
    MouseArea {
        id: dismissOverlay
        enabled: selectorRoot.isOpen
        // Anchor to the root window (using window as parent)
        x: -mapToItem(null, 0, 0).x
        y: -mapToItem(null, 0, 0).y
        width: selectorRoot.Window.width
        height: selectorRoot.Window.height
        z: 99
        onPressed: selectorRoot.isOpen = false
    }

    // Dropdown List
    Rectangle {
        id: dropdown
        visible: selectorRoot.isOpen || opacityAnim.running
        opacity: selectorRoot.isOpen ? 1.0 : 0.0
        scale: selectorRoot.isOpen ? 1.0 : 0.95
        z: 100
        
        width: 180
        height: Math.min(250, (listRepeater.count + 1) * 32 + 8)
        
        anchors.top: button.bottom
        anchors.topMargin: 4
        anchors.right: button.right

        color: "#181825"
        border.color: "#30ffffff"
        border.width: 1
        radius: 8

        Behavior on opacity {
            NumberAnimation { id: opacityAnim; duration: 150; easing.type: Easing.OutQuad }
        }
        Behavior on scale {
            NumberAnimation { duration: 150; easing.type: Easing.OutBack }
        }

        ScrollView {
            anchors.fill: parent
            anchors.margins: 4
            contentWidth: parent.width - 8
            clip: true

            Column {
                id: listColumn
                width: parent.width
                spacing: 2

                // Automatic Option
                Rectangle {
                    width: parent.width
                    height: 30
                    radius: 4
                    color: autoMouse.pressed ? "#25ffffff" : (autoMouse.containsMouse ? "#12ffffff" : "transparent")

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 8

                        Text {
                            text: selectorRoot.preferredPlayerService === "" ? "✓" : ""
                            color: "#00ffff"
                            font.pixelSize: 12
                            font.bold: true
                            width: 12
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "Automatic Switch"
                            color: selectorRoot.preferredPlayerService === "" ? "#00ffff" : "#e0e0e0"
                            font.pixelSize: 11
                            font.family: "Outfit"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        id: autoMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            selectorRoot.playerSelected("");
                            selectorRoot.isOpen = false;
                        }
                    }
                }

                // Discovered Players
                Repeater {
                    id: listRepeater
                    model: selectorRoot.availableServices

                    delegate: Rectangle {
                        width: listColumn.width
                        height: 30
                        radius: 4
                        color: delegateMouse.pressed ? "#25ffffff" : (delegateMouse.containsMouse ? "#12ffffff" : "transparent")

                        readonly property string service: modelData
                        readonly property string identity: selectorRoot.availableIdentities[index] ? selectorRoot.availableIdentities[index] : service

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 8

                            Text {
                                text: selectorRoot.preferredPlayerService === service ? "✓" : ""
                                color: "#ff007f"
                                font.pixelSize: 12
                                font.bold: true
                                width: 12
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: identity
                                color: selectorRoot.preferredPlayerService === service ? "#ff007f" : "#e0e0e0"
                                font.pixelSize: 11
                                font.family: "Outfit"
                                elide: Text.ElideRight
                                width: parent.width - 24
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            id: delegateMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                selectorRoot.playerSelected(service);
                                selectorRoot.isOpen = false;
                            }
                        }
                    }
                }
            }
        }
    }
}
