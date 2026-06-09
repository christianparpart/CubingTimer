import QtQuick
import QtQuick.Controls
import QtCharts

Frame {
    id: root

    function rebuild() {
        series.clear();
        var n = sessionModel.count;
        if (n === 0)
            return;
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

    ChartView {
        anchors.fill: parent
        antialiasing: true
        legend.visible: false
        backgroundColor: "transparent"
        margins.top: 6
        margins.bottom: 6
        margins.left: 6
        margins.right: 6

        ValueAxis { id: axisX; min: 0; max: 1; tickCount: 5; titleText: qsTr("Solve #") }
        ValueAxis { id: axisY; min: 0; max: 1; titleText: qsTr("Seconds") }

        LineSeries {
            id: series
            axisX: axisX
            axisY: axisY
            color: "#1565c0"
            width: 2
        }
    }
}
