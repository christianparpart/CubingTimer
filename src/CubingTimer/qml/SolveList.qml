import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        Label {
            text: qsTr("Solves (%1)").arg(sessionModel.count)
            font.bold: true
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: sessionModel
            delegate: ItemDelegate {
                id: solveRow

                // Explicit, role-typed required properties (the modern Qt 6 form).
                // Unlike the implicit context-object access, these re-evaluate when
                // the model emits dataChanged for the matching role, so editing a
                // solve's penalty (+2 / DNF / OK) repaints its history row in place.
                required property int index
                required property int penalty
                required property var effectiveTimeMs
                required property string scramble

                width: ListView.view ? ListView.view.width : 0
                contentItem: RowLayout {
                    Label {
                        text: "#" + (sessionModel.count - solveRow.index)
                        Layout.preferredWidth: 48
                        opacity: 0.6
                    }
                    Label {
                        text: solveRow.penalty === 2 ? qsTr("DNF") : statsModel.formatMs(solveRow.effectiveTimeMs)
                        font.bold: true
                        Layout.preferredWidth: 100
                    }
                    Label {
                        text: solveRow.penalty === 1 ? qsTr("+2") : ""
                        opacity: 0.7
                    }
                    Label {
                        text: solveRow.scramble
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        opacity: 0.7
                    }
                }
            }
        }

        Button {
            text: qsTr("Delete last solve")
            enabled: sessionModel.count > 0
            onClicked: sessionModel.removeLast()
        }
    }
}
