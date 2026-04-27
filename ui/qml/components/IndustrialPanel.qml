import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtBCI 1.0

Rectangle {
    id: root
    color: Theme.panelBg
    radius: Theme.radiusPanel
    border.width: 1
    border.color: Theme.borderStrong

    Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        radius: Math.max(0, root.radius - 1)
        color: "transparent"
        border.width: 1
        border.color: Theme.borderSoft
    }
}
