import QtQuick

Rectangle {
    id: glassRoot
    radius: 16
    color: "#cc0f0f1d"
    border.color: "#28ffffff"
    border.width: 1

    // Smooth gradient for glassmorphism
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#cc1b1b2d" }
        GradientStop { position: 1.0; color: "#aa0a0a16" }
    }
}
