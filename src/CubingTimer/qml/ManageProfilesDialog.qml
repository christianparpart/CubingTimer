import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Dialog {
    id: dialog
    title: qsTr("Manage profiles")
    modal: true
    standardButtons: Dialog.Close
    width: Math.min(500, parent.width - 40)

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: qsTr("Rename, reorder, or delete profiles. Use the arrows on the right to change the order shown in the toolbar dropdown.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            opacity: 0.7
            font.pixelSize: 12
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(360, contentHeight)
            clip: true
            spacing: 4
            model: profileController.profileNames

            // Track which row is being edited inline.
            property int editingIndex: -1

            delegate: ItemDelegate {
                width: ListView.view ? ListView.view.width : 0
                highlighted: index === profileController.currentProfileId
                            && profileController.profileNames[index] === profileController.currentProfileName
                contentItem: RowLayout {
                    spacing: 6

                    // The name — clickable to edit inline.
                    TextField {
                        id: nameEdit
                        Layout.fillWidth: true
                        text: modelData
                        readOnly: list.editingIndex !== index
                        background: Rectangle {
                            color: list.editingIndex === index ? Material.background : "transparent"
                            border.width: list.editingIndex === index ? 1 : 0
                            border.color: Material.accentColor
                            radius: 2
                        }
                        onAccepted: {
                            profileController.renameProfile(index, text);
                            list.editingIndex = -1;
                        }
                        Keys.onEscapePressed: {
                            text = modelData;
                            list.editingIndex = -1;
                        }
                    }

                    ToolButton {
                        text: list.editingIndex === index ? "✓" : "✎"
                        ToolTip.visible: hovered
                        ToolTip.text: list.editingIndex === index ? qsTr("Save") : qsTr("Rename")
                        onClicked: {
                            if (list.editingIndex === index) {
                                profileController.renameProfile(index, nameEdit.text);
                                list.editingIndex = -1;
                            } else {
                                list.editingIndex = index;
                                nameEdit.forceActiveFocus();
                                nameEdit.selectAll();
                            }
                        }
                    }

                    ToolButton {
                        text: "↑"
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Move up")
                        enabled: index > 0
                        onClicked: profileController.moveProfile(index, index - 1)
                    }

                    ToolButton {
                        text: "↓"
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Move down")
                        enabled: index < (profileController.profileNames.length - 1)
                        onClicked: profileController.moveProfile(index, index + 1)
                    }

                    ToolButton {
                        text: "🗑"
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Delete profile")
                        onClicked: {
                            confirmDelete.targetIndex = index;
                            confirmDelete.targetName = modelData;
                            confirmDelete.open();
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: confirmDelete
        title: qsTr("Delete profile?")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No

        property int targetIndex: -1
        property string targetName: ""

        Label {
            text: qsTr("Delete \"%1\" and all its sessions and solves? This cannot be undone.").arg(confirmDelete.targetName)
            wrapMode: Text.WordWrap
            width: 320
        }
        onAccepted: {
            if (targetIndex >= 0)
                profileController.deleteProfile(targetIndex);
            targetIndex = -1;
            targetName = "";
        }
    }
}
