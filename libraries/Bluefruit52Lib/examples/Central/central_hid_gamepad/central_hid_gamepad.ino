/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/*
 * This sketch demonstrate the central API(). An additional bluefruit
 * that has blehid as peripheral is required for the demo.
 */
#include <bluefruit.h>

// Polling or callback implementation
#define POLLING       1

BLEClientHidGamepad hid;

// Last checked report, to detect if there is changes between reports
hid_gamepad_report_t last_gamepad_report = { 0 };

void setup()
{
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Bluefruit52 Central HID (Gamepad) Example");
  Serial.println("--------------------------------------------------\n");
  
  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);
  
  Bluefruit.setName("Bluefruit52 Central");

  // Init BLE Central Hid Serivce
  hid.begin();

  #if POLLING == 0  
    hid.setGamepadReportCallback(gamepad_report_callback);
  #endif

  // Increase Blink rate to different from PrPh advertising mode
  Bluefruit.setConnLedInterval(250);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  // Set connection secured callback, invoked when connection is encrypted
  Bluefruit.Security.setSecuredCallback(connection_secured_callback);

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Interval = 100 ms, window = 80 ms
   * - Don't use active scan
   * - Filter only accept HID service in advertising
   * - Start(timeout) with timeout = 0 will scan forever (until connected)
   */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.filterService(hid);   // only report HID service
  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0);             // 0 = Don't stop scanning after n seconds
}

/**
 * Callback invoked when scanner pick up an advertising data
 * @param report Structural advertising data
 */
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Since we configure the scanner with filterUuid()
  // Scan callback only invoked for device with hid service advertised  
  // Connect to the device with hid service in advertising packet
  Bluefruit.Central.connect(report);
}

/**
 * Callback invoked when an connection is established
 * @param conn_handle
 */
void connect_callback(uint16_t conn_handle)
{
  BLEConnection* conn = Bluefruit.Connection(conn_handle);

  Serial.println("Connected");

  Serial.print("Discovering HID  Service ... ");

  if ( hid.discover(conn_handle) )
  {
    Serial.println("Found it");

    // HID device mostly require pairing/bonding
    conn->requestPairing();
  }else
  {
    Serial.println("Found NONE");
    
    // disconnect since we couldn't find blehid service
    conn->disconnect();
  }
}

void connection_secured_callback(uint16_t conn_handle)
{
  BLEConnection* conn = Bluefruit.Connection(conn_handle);

  if ( !conn->secured() )
  {
    // It is possible that connection is still not secured by this time.
    // This happens (central only) when we try to encrypt connection using stored bond keys
    // but peer reject it (probably it remove its stored key).
    // Therefore we will request an pairing again --> callback again when encrypted
    conn->requestPairing();
  }
  else
  {
    Serial.println("Secured");

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.hid_information.xml
    uint8_t hidInfo[4];
    hid.getHidInfo(hidInfo);

    Serial.printf("HID version: %d.%d\n", hidInfo[0], hidInfo[1]);
    Serial.print("Country code: "); Serial.println(hidInfo[2]);
    Serial.printf("HID Flags  : 0x%02X\n", hidInfo[3]);

    if (hid.gamepadPresent()) hid.enableGamepad();

    Serial.println("Ready to receive from peripheral");
  }
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;
  
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}

void loop()
{
  
#if POLLING == 1
  // nothing to do if hid not discovered
  if ( !hid.discovered() ) return;
  
  /*------------- Polling Keyboard  -------------*/
  hid_gamepad_report_t gamepad_report;

  // Get latest report
  hid.getGamepadReport(&gamepad_report);

  processGamepadReport(&gamepad_report);


  // polling interval is 5 ms
  delay(5);
#endif
  
}

void gamepad_report_callback(hid_gamepad_report_t* report)
{
    processGamepadReport(report);
}

void processGamepadReport(hid_gamepad_report_t* report) 
{
  // Check with last report to see if there is any changes
  if ( memcmp(&last_gamepad_report, report, sizeof(hid_gamepad_report_t)) )
  {
    if (last_gamepad_report.x != report->x) {
        Serial.print("x: ");Serial.println(report->x);
    }

    if (last_gamepad_report.y != report->y) {
        Serial.print("y: ");Serial.println(report->y);
    }

    if (last_gamepad_report.z != report->z) {
        Serial.print("z: ");Serial.println(report->z);
    }

    if (last_gamepad_report.rz != report->rz) {
        Serial.print("rz: ");Serial.println(report->rz);
    }

    if (last_gamepad_report.rx != report->rx) {
        Serial.print("rx: ");Serial.println(report->rx);
    }

    if (last_gamepad_report.ry != report->ry) {
        Serial.print("ry: ");Serial.println(report->ry);
    }

    if (last_gamepad_report.hat != report->hat) {
        Serial.print("hat: ");Serial.println(report->hat, HEX);
    }

    if (last_gamepad_report.buttons != report->buttons) {
        Serial.print("buttons: ");Serial.println(report->buttons, HEX);
    }
  }

  // update last report
  memcpy(&last_gamepad_report, report, sizeof(hid_gamepad_report_t));    
}
