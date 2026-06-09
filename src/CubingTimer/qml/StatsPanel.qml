import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    GridLayout {
        anchors.fill: parent
        columns: 3
        rowSpacing: 4
        columnSpacing: 16

        Label { text: qsTr("Statistic"); font.bold: true }
        Label { text: qsTr("Current");   font.bold: true }
        Label { text: qsTr("Best");      font.bold: true }

        Label { text: qsTr("Single") }
        Label { text: "" }
        Label { text: statsModel.best }

        Label { text: qsTr("Mo3") }
        Label { text: statsModel.currentMo3 }
        Label { text: statsModel.bestMo3 }

        Label { text: qsTr("Ao5") }
        Label { text: statsModel.currentAo5 }
        Label { text: statsModel.bestAo5 }

        Label { text: qsTr("Ao12") }
        Label { text: statsModel.currentAo12 }
        Label { text: statsModel.bestAo12 }

        Label { text: qsTr("Ao100") }
        Label { text: statsModel.currentAo100 }
        Label { text: statsModel.bestAo100 }

        Item { Layout.columnSpan: 3; Layout.preferredHeight: 8 }

        Label { text: qsTr("90-day"); font.italic: true }
        Label { text: ""; font.italic: true }
        Label { text: statsModel.best90d; font.italic: true }

        Label { text: qsTr("90-day Ao5"); font.italic: true }
        Label { text: ""; font.italic: true }
        Label { text: statsModel.bestAo5_90d; font.italic: true }

        Label { text: qsTr("90-day Ao12"); font.italic: true }
        Label { text: ""; font.italic: true }
        Label { text: statsModel.bestAo12_90d; font.italic: true }
    }
}
