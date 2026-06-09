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
                width: ListView.view ? ListView.view.width : 0
                contentItem: RowLayout {
                    Label {
                        text: "#" + (sessionModel.count - index)
                        Layout.preferredWidth: 48
                        opacity: 0.6
                    }
                    Label {
                        text: penalty === 2 ? "DNF" : statsModel.formatMs(effectiveTimeMs)
                        font.bold: true
                        Layout.preferredWidth: 100
                    }
                    Label {
                        text: penalty === 1 ? qsTr("+2") : ""
                        opacity: 0.7
                    }
                    Label {
                        text: scramble
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
