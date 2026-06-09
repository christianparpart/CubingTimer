import QtQuick
import QtQuick.Controls
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
            var eff = sessionModel.data(idx, 0x101 + 2); // EffectiveTimeMsRole
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
        anchors.fill: parent

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
            color: "#1565c0"
            width: 2
        }
    }
}
