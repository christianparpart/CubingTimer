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

    Component.onCompleted: {
        // Configure the default theme imperatively. Declaring an inline
        // `theme: GraphsTheme { ... }` fails on the Qt 6.7 WASM build with
        // "GraphsTheme is not a type" because that QML type isn't exported
        // there, even though the C++ side and desktop QML do know it.
        var t = view.theme;
        if (t) {
            try {
                t.colorScheme = 1; // GraphsTheme.ColorScheme.Light
                t.backgroundColor = "white";
                t.plotAreaBackgroundColor = "white";
                if (t.grid) {
                    t.grid.mainColor = "#e0e0e0";
                    t.grid.subColor = "#f0f0f0";
                    t.grid.mainWidth = 1;
                    t.grid.subWidth = 1;
                }
                if (t.axisX) {
                    t.axisX.mainColor = "#bdbdbd";
                    t.axisX.mainWidth = 1;
                }
                if (t.axisY) {
                    t.axisY.mainColor = "#bdbdbd";
                    t.axisY.mainWidth = 1;
                }
            } catch (e) {
                // Older/newer QtGraphs may name these differently; we'd
                // rather render with default styling than crash here.
                console.warn("HistoryGraph theme tweak failed:", e);
            }
        }
        rebuild();
    }

    GraphsView {
        id: view
        anchors.fill: parent
        marginTop: 8
        marginBottom: 4
        marginLeft: 4
        marginRight: 4

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
