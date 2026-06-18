import QtQuick
import QtQuick.Effects

Item {
    id: recordRoot
    width: 380
    height: 380

    property string coverUrl: ""
    property bool isPlaying: false
    property real progress: 0.0 // 0.0 to 1.0

    // Physics Controller
    QtObject {
        id: physics
        property real currentSpeed: 0.0 // degrees/sec
        property real targetSpeed: recordRoot.isPlaying ? 99.8 : 0.0 // 33.3 RPM = 199.8 deg/sec
        property real angle: 0.0
    }

    // GPU-synchronized FrameAnimation for 60fps smooth rotation
    FrameAnimation {
        running: recordRoot.isPlaying || physics.currentSpeed > 0.05
        onTriggered: {
            // Frame rate independent smooth speed interpolation
            var alpha = 1.0 - Math.exp(-2.5 * frameTime);
            physics.currentSpeed += (physics.targetSpeed - physics.currentSpeed) * alpha;
            physics.angle = (physics.angle + physics.currentSpeed * frameTime) % 360;
        }
    }

    // Outer Vinyl Shadow
    Rectangle {
        id: vinylShadow
        anchors.fill: recordGroup
        radius: width / 2
        color: "transparent"
        border.color: "#0c000000"
        border.width: 1
        
        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: "#80000000"
            shadowBlur: 20
            shadowVerticalOffset: 8
        }
    }

    // Rotating Group (Disc + Cover)
    Item {
        id: recordGroup
        width: parent.width - 40
        height: width
        anchors.centerIn: parent
        rotation: physics.angle

        // Procedural Vinyl Grooves Canvas
        Canvas {
            id: vinylCanvas
            anchors.fill: parent
            
            onPaint: {
                var ctx = getContext("2d");
                ctx.reset();
                
                var cx = width / 2;
                var cy = height / 2;
                var r = width / 2;
                
                // 1. Base Dark Vinyl Plate
                ctx.beginPath();
                ctx.arc(cx, cy, r - 2, 0, 2 * Math.PI);
                ctx.fillStyle = "#0a0a0f";
                ctx.fill();
                
                // Outer ring lip
                ctx.beginPath();
                ctx.arc(cx, cy, r - 3, 0, 2 * Math.PI);
                ctx.strokeStyle = "#1b1b22";
                ctx.lineWidth = 3;
                ctx.stroke();

                // 2. Concentric Grooves (extremely subtle highlights)
                ctx.strokeStyle = "rgba(255, 255, 255, 0.02)";
                ctx.lineWidth = 0.5;
                for (var i = r * 0.36; i < r - 12; i += 2.5) {
                    ctx.beginPath();
                    ctx.arc(cx, cy, i, 0, 2 * Math.PI);
                    ctx.stroke();
                }

                // 3. Matte Track Separator Rings
                ctx.strokeStyle = "rgba(0, 0, 0, 0.3)";
                ctx.lineWidth = 1.5;
                for (var j = r * 0.5; j < r - 15; j += 25) {
                    ctx.beginPath();
                    ctx.arc(cx, cy, j, 0, 2 * Math.PI);
                    ctx.stroke();
                }
            }
        }

        // Mask source for circular clipping (invisible, used by MultiEffect)
        Rectangle {
            id: circularClipMask
            width: parent.width * 0.90
            height: width
            radius: width / 2
            color: "white"
            visible: false
            layer.enabled: true
        }

        // Circular Album Art Label
        Rectangle {
            id: labelContainer
            width: parent.width * 0.90
            height: width
            radius: width / 2
            color: "#12121c"
            anchors.centerIn: parent

            layer.enabled: true
            layer.effect: MultiEffect {
                maskEnabled: true
                maskThresholdMin: 0.5
                maskSpreadAtMin: 1.0
                maskSource: circularClipMask
            }

            Image {
                id: albumArt
                source: recordRoot.coverUrl ? recordRoot.coverUrl : "../../assets/default_cover.png"
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                smooth: true
                asynchronous: true

                // Fade transition when track changes
                Behavior on source {
                    SequentialAnimation {
                        NumberAnimation { target: albumArt; property: "opacity"; to: 0; duration: 200 }
                        PropertyAction { target: albumArt; property: "opacity"; value: 0 }
                        PropertyAction { } // Perform actual source change
                        NumberAnimation { target: albumArt; property: "opacity"; to: 1.0; duration: 250 }
                    }
                }
            }

            // Stylized Metal Spindle Ring in very center
            Rectangle {
                width: 14
                height: 14
                radius: 7
                color: "#18181c"
                border.color: "#80a0a0a0"
                border.width: 1.5
                anchors.centerIn: parent

                // Spindle Hole
                Rectangle {
                    width: 6
                    height: 6
                    radius: 3
                    color: "#000000"
                    anchors.centerIn: parent
                }
            }
        }
    }

    // STATIC Specular Shine Cones (Do not rotate! Realistic reflection effect)
    Canvas {
        id: specularShine
        width: recordGroup.width
        height: width
        anchors.centerIn: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            var cx = width / 2;
            var cy = height / 2;
            var r = width / 2;

            // Two soft light reflection wedges (opposite angle cones)
            var grad = ctx.createRadialGradient(cx, cy, r * 0.35, cx, cy, r);
            grad.addColorStop(0, "rgba(255, 255, 255, 0.05)");
            grad.addColorStop(0.5, "rgba(255, 255, 255, 0.12)");
            grad.addColorStop(1, "rgba(255, 255, 255, 0.01)");
            
            ctx.fillStyle = grad;

            // Top-Right Wedge
            ctx.beginPath();
            ctx.moveTo(cx, cy);
            ctx.arc(cx, cy, r - 6, -Math.PI / 4 - 0.25, -Math.PI / 4 + 0.25);
            ctx.closePath();
            ctx.fill();

            // Bottom-Left Wedge
            ctx.beginPath();
            ctx.moveTo(cx, cy);
            ctx.arc(cx, cy, r - 6, 3 * Math.PI / 4 - 0.25, 3 * Math.PI / 4 + 0.25);
            ctx.closePath();
            ctx.fill();
        }
    }

    // Stylus Arm (Pivoting tonearm visualizer)
    Item {
        id: tonearmPivot
        x: parent.width - 50
        y: 40
        width: 1
        height: 1

        // Angles:
        // -30: rest position
        // -5 to 12: active playback tracking outer to inner grooves
        readonly property real restAngle: -32.0
        readonly property real activeStartAngle: -10.0
        readonly property real activeEndAngle: 12.0
        readonly property real targetAngle: recordRoot.isPlaying ? 
            (activeStartAngle + recordRoot.progress * (activeEndAngle - activeStartAngle)) : restAngle

        rotation: targetAngle

        Behavior on rotation {
            NumberAnimation {
                duration: recordRoot.isPlaying ? 800 : 1500
                easing.type: Easing.OutCubic
            }
        }

        // Visual Tonearm Body
        Item {
            anchors.centerIn: parent
            rotation: 0

            // The main long metallic stem
            Rectangle {
                width: 6
                height: 160
                color: "#e0e0e0"
                radius: 3
                x: -3
                y: -10 // Pivot offset

                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: "#cccccc" }
                    GradientStop { position: 0.5; color: "#ffffff" }
                    GradientStop { position: 1.0; color: "#999999" }
                }
            }

            // Pivot head decoration
            Rectangle {
                width: 26
                height: 26
                radius: 13
                color: "#202028"
                border.color: "#80777777"
                border.width: 2
                anchors.centerIn: parent
                
                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: "#111111"
                    anchors.centerIn: parent
                }
            }

            // Tonearm Head / Cartridge Stylus
            Item {
                x: -6
                y: 140 // Near the end of the stem

                // Cartridge body
                Rectangle {
                    width: 12
                    height: 24
                    color: "#18181c"
                    radius: 2
                    rotation: 10 // Angled stylus cartridge

                    // Brand highlight strip
                    Rectangle {
                        width: parent.width
                        height: 2
                        color: "#ff007f"
                        anchors.top: parent.top
                        anchors.topMargin: 4
                    }
                }
            }
        }
    }
}
