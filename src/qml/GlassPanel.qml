import QtQuick

Rectangle {
    id: glassRoot
    radius: 16
    color: "#1e0f0f1d"
    border.color: "#28ffffff"
    border.width: 1

    // Smooth gradient for glassmorphism
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#2b1b1b2d" }
        GradientStop { position: 1.0; color: "#140a0a16" }
    }
}
