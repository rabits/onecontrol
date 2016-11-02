import QtQuick 2.7
import QtQuick.Controls 2.0
import org.rabits.onecontrol 1.0
import QtWebView 1.1
import QtQuick.Layouts 1.3

ApplicationWindow {
    id: window
    title: "OneControl"
    width: 360
    height: 640
    visible: true

    header: Rectangle {
        height: window.height / 15
        color: "#000000"
        Text {
            id: device_name
            anchors.fill: parent

            color: "#eeeeee"
            text: qsTr("Device not selected")

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }

    StackLayout {
        id: swipe
        anchors.fill: parent
        currentIndex: tabbar.currentIndex

        Rectangle {
            id: onebutton_tab
            color: "#aa0000"

            JsonRPC {
                id: onebutton_jsonrpc
            }
            Column {
                enabled: onebutton_jsonrpc.address_port != ""
                Button {
                    id: service_poweroff
                    width: 100
                    height: 100
                    text: qsTr("Power off")
                    onClicked: {
                        onebutton_jsonrpc.requestCallback("system.poweroff", [], function(result){
                            console.log(JSON.stringify(result))
                        })
                    }
                }
                Button {
                    id: onebutton_restart
                    width: 100
                    height: 100
                    text: qsTr("Restart")
                    onClicked: {
                        onebutton_jsonrpc.requestCallback("onebutton.restart", [], function(result){
                            console.log(JSON.stringify(result))
                        })
                    }
                }
            }
        }
        Rectangle {
            id: guitarix_tab
            color: "#00aa00"
            WebView {
                id: guitarix_webview
                anchors.fill: parent
                url: ""
            }
        }
        Rectangle {
            id: ssh_tab
            color: "#aa0000"
            Text {
                id: ssh_service_header
                anchors.fill: parent
                anchors.bottomMargin: parent.height/2

                verticalAlignment: Text.AlignBottom
                horizontalAlignment: Text.AlignHCenter

                font.bold: true
                text: qsTr("SSH Service")
            }
            Text {
                id: ssh_service_text
                anchors.fill: parent
                anchors.topMargin: parent.height/2

                verticalAlignment: Text.AlignTop
                horizontalAlignment: Text.AlignHCenter

                text: qsTr("Disabled")
            }
        }
    }

    SelectDevice {
        id: select_device
        anchors.fill: parent
        visible: true

        property string name: ""
        property string address: ""
        property bool connected: false

        onDeviceSelected: {
            select_device.name = name
            select_device.address = address
            app.bluetooth().connectTo(address)
            device_name.text = qsTr("Connecting ") + name + " (" + address + ")"
        }

        Connections {
            target: app.bluetooth()
            onStateChanged: {
                device_name.text = (connected ? qsTr("Connected") : qsTr("Disconnected")) + " " + select_device.name + " (" + select_device.address + ")"
                select_device.connected = connected
                select_device.visible = ! connected
                if( connected ) {
                    cfg.setting('bluetooth/last_device_name', select_device.name)
                    cfg.setting('bluetooth/last_device_address', select_device.address)
                }
            }
            onAvailableServices: {
                // Update onebutton address
                onebutton_jsonrpc.address_port = app.bluetooth().getServiceAddress("OneControl")
            }
        }
    }

    footer: Item {
        height: window.height / 12

        TabBar {
            id: tabbar
            anchors.left: parent.left
            anchors.right: menu_button.left
            currentIndex: swipe.currentIndex

            width: parent.width
            height: parent.height
            TabButton {
                text: qsTr("OneButton")
                enabled: select_device.connected
                height: parent.height
                onClicked: {
                }
            }
            TabButton {
                text: qsTr("Guitarix")
                enabled: select_device.connected
                height: parent.height
                onClicked: {
                    guitarix_webview.url = "http://" + app.bluetooth().getServiceAddress("Guitarix WEB") + "/"
                }
            }
            TabButton {
                text: qsTr("SSH")
                enabled: select_device.connected
                height: parent.height
                onClicked: {
                    var address = app.bluetooth().getServiceAddress("SSH")
                    if( address !== "" )
                        ssh_service_text.text = qsTr("Available at address:") + " " + address
                }
            }
        }
        Button {
            id: menu_button

            anchors.right: parent.right

            text: "..."
            onClicked: menu.open()
            height: parent.height
            width: height

            Menu {
                id: menu
                x: menu_button.width - width

                onVisibleChanged: {
                    // Fix for WebView overlapping issue
                    // http://doc.qt.io/qt-5/qml-qtwebview-webview.html#details
                    if( guitarix_webview.visible )
                        guitarix_webview.visible = ! visible
                }

                MenuItem {
                    text: "Select Device"
                    onClicked: select_device.visible = !select_device.visible
                }
            }
        }
    }

    Component.onCompleted: {
        if( cfg.setting('bluetooth/last_device_address') ) {
            select_device.deviceSelected(cfg.setting('bluetooth/last_device_name'),
                                         cfg.setting('bluetooth/last_device_address'))
        }
    }
}
