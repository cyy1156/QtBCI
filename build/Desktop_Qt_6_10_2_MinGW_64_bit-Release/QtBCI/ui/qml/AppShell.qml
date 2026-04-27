import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtBCI 1.0

ApplicationWindow {
    visible: true
    width: 1366
    height: 860
    minimumWidth: 1200
    minimumHeight: 760
    title: "QtBCI 工业上位机"
    color: Theme.windowBg

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingL
        spacing: Theme.spacingL

        IndustrialPanel {
            Layout.preferredWidth: 220
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacingL
                spacing: Theme.spacingM

                SectionTitle {
                    text: "控制面板"
                    Layout.alignment: Qt.AlignHCenter
                }

                TextField {
                    text: uiBridge.portName
                    placeholderText: "端口名，例如 COM7"
                    onEditingFinished: uiBridge.portName = text
                    Layout.fillWidth: true
                }

                IndustrialButton {
                    text: "开始"
                    Layout.fillWidth: true
                    onClicked: uiBridge.start()
                }
                IndustrialButton {
                    text: "停止"
                    Layout.fillWidth: true
                    onClicked: uiBridge.stop()
                }
                IndustrialButton {
                    text: "清除"
                    Layout.fillWidth: true
                    onClicked: uiBridge.clear()
                }
                IndustrialButton {
                    text: "保存"
                    Layout.fillWidth: true
                    onClicked: uiBridge.openSettings()
                }
                Item { Layout.fillHeight: true }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacingM

            IndustrialPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 360

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.spacingM
                    spacing: Theme.spacingM

                    SectionTitle { text: "实时脑电波形" }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: Theme.panelBgLight
                        border.width: 1
                        border.color: Theme.borderSoft
                        radius: 6

                        Canvas {
                            id: waveformCanvas
                            anchors.fill: parent
                            anchors.margins: 6

                            onPaint: {
                                const ctx = getContext("2d");
                                ctx.reset();
                                ctx.fillStyle = Theme.panelBgLight;
                                ctx.fillRect(0, 0, width, height);
                                const data = uiBridge.plotPoints;
                                if (!data || data.length < 2)
                                    return;
                                ctx.strokeStyle = "#3de0bc";
                                ctx.lineWidth = 1.2;
                                ctx.beginPath();
                                for (let i = 0; i < data.length; ++i) {
                                    const x = i * width / (data.length - 1);
                                    const v = Number(data[i]);
                                    const y = height * 0.5 - v * 1.5;
                                    if (i === 0)
                                        ctx.moveTo(x, y);
                                    else
                                        ctx.lineTo(x, y);
                                }
                                ctx.stroke();
                            }

                            Connections {
                                target: uiBridge
                                function onPlotPointsChanged() {
                                    waveformCanvas.requestPaint();
                                }
                            }
                        }
                    }
                }
            }

            IndustrialPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.spacingM
                    spacing: Theme.spacingM

                    SectionTitle { text: "系统日志" }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: Theme.panelBgLight
                        border.width: 1
                        border.color: Theme.borderSoft
                        radius: 6

                        ListView {
                            id: logList
                            anchors.fill: parent
                            anchors.margins: 8
                            model: uiBridge.logs
                            clip: true

                            delegate: Text {
                                required property string modelData
                                text: modelData
                                color: Theme.textPrimary
                                font.pixelSize: 14
                            }

                            Connections {
                                target: uiBridge
                                function onLogsChanged() {
                                    logList.positionViewAtEnd();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
