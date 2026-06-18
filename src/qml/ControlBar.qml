import QtQuick

Item {
    id: controlRoot
    height: 60

    property bool isPlaying: false
    property bool canGoNext: true
    property bool canGoPrevious: true
    property bool canPlay: true
    property bool canPause: true

    signal prevClicked()
    signal playPauseClicked()
    signal nextClicked()

    Row {
        anchors.centerIn: parent
        spacing: 24

        // Previous Button
        Rectangle {
            id: prevButton
            width: 40
            height: 40
            radius: 20
            color: prevMouse.pressed ? "#30ffffff" : (prevMouse.containsMouse ? "#1affffff" : "#08ffffff")
            border.color: prevMouse.containsMouse ? "#a000ffff" : "#1affffff"
            border.width: 1
            opacity: controlRoot.canGoPrevious ? 1.0 : 0.4
            scale: prevMouse.pressed ? 0.92 : (prevMouse.containsMouse ? 1.05 : 1.0)

            Behavior on color { ColorAnimation { duration: 150 } }
            Behavior on border.color { ColorAnimation { duration: 150 } }
            Behavior on scale { NumberAnimation { duration: 100 } }

            Text {
                text: "⏮"
                color: prevMouse.containsMouse ? "#00ffff" : "#ffffff"
                font.pixelSize: 16
                anchors.centerIn: parent
            }

            MouseArea {
                id: prevMouse
                anchors.fill: parent
                hoverEnabled: controlRoot.canGoPrevious
                enabled: controlRoot.canGoPrevious
                cursorShape: Qt.PointingHandCursor
                onClicked: controlRoot.prevClicked()
            }
        }

        // Play/Pause Button
        Rectangle {
            id: playPauseButton
            width: 54
            height: 54
            radius: 27
            color: playPauseMouse.pressed ? "#40ffffff" : (playPauseMouse.containsMouse ? "#20ffffff" : "#10ffffff")
            border.color: playPauseMouse.containsMouse ? "#ff007f" : "#33ffffff"
            border.width: 1.5
            scale: playPauseMouse.pressed ? 0.90 : (playPauseMouse.containsMouse ? 1.08 : 1.0)
            opacity: (controlRoot.isPlaying ? controlRoot.canPause : controlRoot.canPlay) ? 1.0 : 0.4

            // Radial-like glowing shadow when hovering/playing
            layer.enabled: playPauseMouse.containsMouse || controlRoot.isPlaying
            
            Behavior on color { ColorAnimation { duration: 150 } }
            Behavior on border.color { ColorAnimation { duration: 150 } }
            Behavior on scale { NumberAnimation { duration: 100 } }

            Text {
                text: controlRoot.isPlaying ? "⏸" : "▶"
                color: playPauseMouse.containsMouse ? "#ff007f" : "#ffffff"
                font.pixelSize: controlRoot.isPlaying ? 20 : 22
                // Minor adjustment to offset the play triangle center alignment
                anchors.centerIn: parent
                anchors.horizontalCenterOffset: (!controlRoot.isPlaying) ? 2 : 0
            }

            MouseArea {
                id: playPauseMouse
                anchors.fill: parent
                hoverEnabled: true
                enabled: controlRoot.isPlaying ? controlRoot.canPause : controlRoot.canPlay
                cursorShape: Qt.PointingHandCursor
                onClicked: controlRoot.playPauseClicked()
            }
        }

        // Next Button
        Rectangle {
            id: nextButton
            width: 40
            height: 40
            radius: 20
            color: nextMouse.pressed ? "#30ffffff" : (nextMouse.containsMouse ? "#1affffff" : "#08ffffff")
            border.color: nextMouse.containsMouse ? "#a000ffff" : "#1affffff"
            border.width: 1
            opacity: controlRoot.canGoNext ? 1.0 : 0.4
            scale: nextMouse.pressed ? 0.92 : (nextMouse.containsMouse ? 1.05 : 1.0)

            Behavior on color { ColorAnimation { duration: 150 } }
            Behavior on border.color { ColorAnimation { duration: 150 } }
            Behavior on scale { NumberAnimation { duration: 100 } }

            Text {
                text: "⏭"
                color: nextMouse.containsMouse ? "#00ffff" : "#ffffff"
                font.pixelSize: 16
                anchors.centerIn: parent
            }

            MouseArea {
                id: nextMouse
                anchors.fill: parent
                hoverEnabled: controlRoot.canGoNext
                enabled: controlRoot.canGoNext
                cursorShape: Qt.PointingHandCursor
                onClicked: controlRoot.nextClicked()
            }
        }
    }
}
