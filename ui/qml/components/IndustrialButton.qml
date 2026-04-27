import QtQuick 2.15
import QtQuick.Controls 2.15
import QtBCI 1.0

Button {
    id: root
    implicitWidth: 136
    implicitHeight: 48

    font.pixelSize: 24

    background: Rectangle {
        radius: Theme.radiusButton
        border.width: 1
        border.color: Theme.borderStrong
        color: {
            if (!root.enabled) return "#1c2330";
            if (root.down) return Theme.accentPressed;
            if (root.hovered) return "#3f8ce0";
            return Theme.accent;
        }
    }

    contentItem: Text {
        text: root.text
        color: Theme.textPrimary
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font: root.font
    }
}
