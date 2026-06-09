import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtGraphs

Frame {
    id: root

    function rebuild() {
        series.clear();
        var n = sessionModel.count;
        if (n === 0) {
            axisX.max = 1;
            axisY.max = 1;
            return;
        }
        var max = 0;
        for (var i = 0; i < n; ++i) {
            var idx = sessionModel.index(i, 0);
            // EffectiveTimeMsRole — keep the magic number in lockstep with
            // SessionModel::Roles::EffectiveTimeMsRole (Qt::UserRole + 3).
            var eff = sessionModel.data(idx, Qt.UserRole + 3);
            if (eff < 0)
                continue;
            var seconds = eff / 1000.0;
            series.append(i + 1, seconds);
            if (seconds > max) max = seconds;
        }
        axisX.max = Math.max(1, n);
        axisY.max = max > 0 ? max * 1.1 : 1;
    }

    Connections {
        target: sessionModel
        function onSolvesChanged() { root.rebuild(); }
    }

    Component.onCompleted: rebuild()

    GraphsView {
        id: view
        anchors.fill: parent
        marginTop: 8
        marginBottom: 4
        marginLeft: 4
        marginRight: 4

        // Light theme to match the rest of the app (Material light). Without
        // this the graph defaults to a dark grid + axes on a dark background.
        // We also lighten the grid lines — the default mainWidth=2 with a
        // mid-grey colour reads heavier than the data line itself.
        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Light
            backgroundColor: "white"
            plotAreaBackgroundColor: "white"
            grid {
                mainColor: "#e0e0e0"
                subColor: "#f0f0f0"
                mainWidth: 1
                subWidth: 1
            }
            axisX {
                mainColor: "#bdbdbd"
                mainWidth: 1
            }
            axisY {
                mainColor: "#bdbdbd"
                mainWidth: 1
            }
        }

        axisX: ValueAxis {
            id: axisX
            min: 0
            max: 1
            subTickCount: 1
            titleText: qsTr("Solve #")
        }
        axisY: ValueAxis {
            id: axisY
            min: 0
            max: 1
            titleText: qsTr("Seconds")
        }

        LineSeries {
            id: series
            color: Material.color(Material.Blue, Material.Shade700)
            width: 2
        }
    }
}
