import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Item {
    id: root
    focus: true

    function stateColor(s) {
        switch (s) {
            case 1: return "#fff9c4"; // InspectionPending
            case 2: return "#fff59d"; // InspectionRunning
            case 3: return "#ffcdd2"; // HoldPending — red
            case 4: return "#c8e6c9"; // Armed — green
            case 5: return "#ffffff"; // Running
            case 6: return "#eceff1"; // Stopped
            default: return "#fafafa"; // Idle
        }
    }

    function formatElapsed(ms) {
        return statsModel.formatMs(ms);
    }

    // Scale the big time display so it always fits the available width.
    // ~ width / 5 puts ~5 characters across a typical view; clamp so it neither
    // disappears on narrow phones nor swamps a wide desktop window.
    readonly property real bigTimePx: Math.max(48, Math.min(width / 5, height / 3.5, 200))
    readonly property real scramblePx: Math.max(14, Math.min(width / 30, 24))

    Rectangle {
        anchors.fill: parent
        color: stateColor(timerController.state)

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Label {
                Layout.fillWidth: true
                text: scrambleProvider.current
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: root.scramblePx
            }

            Item { Layout.fillHeight: true }

            Label {
                id: bigTime
                Layout.alignment: Qt.AlignHCenter
                font.pixelSize: root.bigTimePx
                font.bold: true
                fontSizeMode: Text.HorizontalFit
                text: timerController.inspectionEnabled
                      && (timerController.state === 2 || timerController.state === 3 || timerController.state === 4)
                      ? Math.max(0, 15 - Math.floor(timerController.inspectionElapsedMs / 1000)).toString()
                      : formatElapsed(timerController.elapsedMs)
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                opacity: 0.7
                text: timerController.state === 0 ? qsTr("Hold SPACE (or tap) to start")
                       : timerController.state === 3 ? qsTr("Keep holding…")
                       : timerController.state === 4 ? qsTr("Release to start")
                       : timerController.state === 5 ? qsTr("Press any key to stop")
                       : timerController.state === 6 ? qsTr("Solved! Press SPACE for the next")
                       : ""
            }

            Item { Layout.fillHeight: true }

            // Flow wraps to a second line on narrow widths so nothing clips.
            Flow {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                spacing: 8

                Button {
                    text: qsTr("+2")
                    enabled: timerController.state === 6
                    onClicked: sessionModel.setLastPenalty(1)
                }
                Button {
                    text: qsTr("DNF")
                    enabled: timerController.state === 6
                    onClicked: sessionModel.setLastPenalty(2)
                }
                Button {
                    text: qsTr("OK")
                    enabled: timerController.state === 6
                    onClicked: sessionModel.setLastPenalty(0)
                }
                CheckBox {
                    text: qsTr("Inspection")
                    checked: timerController.inspectionEnabled
                    onToggled: timerController.inspectionEnabled = checked
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onPressed: handlePress()
            onReleased: handleRelease()
        }
    }

    function handlePress() {
        if (timerController.state === 5) {
            timerController.stop();
        } else if (timerController.state === 6) {
            timerController.reset();
        } else {
            timerController.holdBegin();
        }
    }

    function handleRelease() {
        timerController.holdEnd();
    }

    Keys.onPressed: (event) => {
        if (event.isAutoRepeat)
            return;
        if (event.key === Qt.Key_Space) {
            handlePress();
            event.accepted = true;
        } else if (event.key === Qt.Key_Escape) {
            timerController.reset();
            event.accepted = true;
        } else if (timerController.state === 5) {
            timerController.stop();
            event.accepted = true;
        }
    }

    Keys.onReleased: (event) => {
        if (event.isAutoRepeat)
            return;
        if (event.key === Qt.Key_Space) {
            handleRelease();
            event.accepted = true;
        }
    }
}
