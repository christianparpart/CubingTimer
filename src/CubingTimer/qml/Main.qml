import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: window
    width: 1100
    height: 720
    minimumWidth: 380
    minimumHeight: 480
    visible: true
    title: qsTr("CubingTimer")

    // Single visual theme: Material Light across all controls + charts.
    Material.theme: Material.Light
    Material.primary: Material.Indigo
    Material.accent: Material.Indigo
    color: Material.backgroundColor

    // Reflow threshold: stack the right-hand panels under the timer when the
    // window is narrower than this. The number is chosen so that on phone-sized
    // viewports the timer gets the full width and stats/graph/list flow below.
    readonly property bool narrow: width < 820

    header: ToolBar {
        Material.elevation: 2
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 8

            Label {
                text: qsTr("Profile")
            }
            ComboBox {
                id: profileBox
                Layout.preferredWidth: window.narrow ? 120 : 180
                model: profileController.profileNames
                onActivated: profileController.selectProfile(currentIndex)
            }
            ToolButton {
                icon.source: ""
                text: "+"
                ToolTip.visible: hovered
                ToolTip.text: qsTr("New profile")
                onClicked: newProfileDialog.open()
            }
            ToolButton {
                text: "⚙"
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Manage profiles")
                onClicked: manageProfilesDialog.open()
            }

            Item { Layout.preferredWidth: window.narrow ? 8 : 24 }

            Label {
                text: qsTr("Session")
                visible: !window.narrow
            }
            ComboBox {
                id: sessionBox
                Layout.preferredWidth: window.narrow ? 110 : 200
                textRole: "name"
                model: profileController.sessions
                onActivated: profileController.selectSession(currentIndex)
            }
            ToolButton {
                text: "+"
                ToolTip.visible: hovered
                ToolTip.text: qsTr("New session")
                onClicked: newSessionDialog.open()
            }

            Item { Layout.fillWidth: true }

            Label {
                visible: !window.narrow
                text: qsTr("Puzzle: ") + scrambleProvider.puzzle
                opacity: 0.7
            }
        }
    }

    // Wide layout: side-by-side, with a resizable splitter.
    Loader {
        anchors.fill: parent
        active: !window.narrow
        sourceComponent: wideLayout
    }
    // Narrow layout: single column, stats panel collapses under the timer.
    Loader {
        anchors.fill: parent
        active: window.narrow
        sourceComponent: narrowLayout
    }

    Component {
        id: wideLayout
        SplitView {
            orientation: Qt.Horizontal

            TimerScreen {
                SplitView.fillWidth: true
                SplitView.minimumWidth: 380
            }

            ColumnLayout {
                SplitView.preferredWidth: 360
                SplitView.minimumWidth: 280
                spacing: 0

                StatsPanel { Layout.fillWidth: true }
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
    }

    Component {
        id: narrowLayout
        Flickable {
            contentWidth: width
            contentHeight: stack.implicitHeight
            clip: true
            ColumnLayout {
                id: stack
                width: parent.width
                spacing: 0

                TimerScreen {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.max(360, window.height * 0.55)
                }
                StatsPanel { Layout.fillWidth: true }
                HistoryGraph {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                }
                SolveList {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.max(160, window.height * 0.3)
                }
            }
        }
    }

    // ---- Dialogs ---------------------------------------------------------

    Dialog {
        id: newProfileDialog
        title: qsTr("New profile")
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        TextField {
            id: profileNameField
            placeholderText: qsTr("name")
            width: 280
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
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        ColumnLayout {
            TextField {
                id: sessionNameField
                placeholderText: qsTr("name")
                Layout.preferredWidth: 280
            }
            ComboBox {
                id: sessionPuzzleBox
                model: [ "333", "222", "444" ]
                Layout.preferredWidth: 280
            }
        }
        onAccepted: {
            if (sessionNameField.text.length > 0)
                profileController.createSession(sessionNameField.text, sessionPuzzleBox.currentText);
            sessionNameField.text = "";
        }
    }

    ManageProfilesDialog {
        id: manageProfilesDialog
        anchors.centerIn: parent
    }
}
