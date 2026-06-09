import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: window
    width: 1100
    height: 720
    visible: true
    title: qsTr("CubingTimer")

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12

            Label {
                text: qsTr("Profile:")
            }
            ComboBox {
                id: profileBox
                Layout.preferredWidth: 160
                model: profileController.profileNames
                onActivated: profileController.selectProfile(currentIndex)
            }
            Button {
                text: qsTr("+ Profile")
                onClicked: newProfileDialog.open()
            }

            Item { Layout.preferredWidth: 24 }

            Label {
                text: qsTr("Session:")
            }
            ComboBox {
                id: sessionBox
                Layout.preferredWidth: 220
                textRole: "name"
                model: profileController.sessions
                onActivated: profileController.selectSession(currentIndex)
            }
            Button {
                text: qsTr("+ Session")
                onClicked: newSessionDialog.open()
            }

            Item { Layout.fillWidth: true }

            Label { text: qsTr("Puzzle: ") + scrambleProvider.puzzle }
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        TimerScreen {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 380
        }

        ColumnLayout {
            SplitView.preferredWidth: 360
            SplitView.minimumWidth: 280
            spacing: 0

            StatsPanel {
                Layout.fillWidth: true
            }
            HistoryGraph {
                Layout.fillWidth: true
                Layout.preferredHeight: 220
            }
            SolveList {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    Dialog {
        id: newProfileDialog
        title: qsTr("New profile")
        standardButtons: Dialog.Ok | Dialog.Cancel
        TextField {
            id: profileNameField
            placeholderText: qsTr("name")
            width: 240
        }
        onAccepted: {
            if (profileNameField.text.length > 0)
                profileController.createProfile(profileNameField.text);
            profileNameField.text = "";
        }
    }

    Dialog {
        id: newSessionDialog
        title: qsTr("New session")
        standardButtons: Dialog.Ok | Dialog.Cancel
        ColumnLayout {
            TextField {
                id: sessionNameField
                placeholderText: qsTr("name")
                Layout.preferredWidth: 240
            }
            ComboBox {
                id: sessionPuzzleBox
                model: [ "333", "222", "444" ]
                Layout.preferredWidth: 240
            }
        }
        onAccepted: {
            if (sessionNameField.text.length > 0)
                profileController.createSession(sessionNameField.text, sessionPuzzleBox.currentText);
            sessionNameField.text = "";
        }
    }
}
