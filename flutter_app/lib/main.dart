import 'dart:async';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'package:permission_handler/permission_handler.dart';

void main() => runApp(const RobotApp());

class RobotApp extends StatelessWidget {
  const RobotApp({super.key});
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Robot Controller',
      debugShowCheckedModeBanner: false,
      theme: ThemeData.dark().copyWith(
        scaffoldBackgroundColor: const Color(0xFF0A0A0A),
      ),
      home: const ControllerPage(),
    );
  }
}

class ControllerPage extends StatefulWidget {
  const ControllerPage({super.key});
  @override
  State<ControllerPage> createState() => _ControllerPageState();
}

class _ControllerPageState extends State<ControllerPage> {
  BluetoothConnection? connection;
  bool isConnected = false;
  String status = 'Not Connected';
  String currentSpeed = '5';

  Timer? _holdTimer;

  @override
  void initState() {
    super.initState();
    _requestPermissions();
  }

  Future<void> _requestPermissions() async {
    await [
      Permission.bluetooth,
      Permission.bluetoothConnect,
      Permission.bluetoothScan,
      Permission.location,
    ].request();
  }

  void _sendCommand(String cmd) {
    if (isConnected && connection != null) {
      connection!.output.add(Uint8List.fromList(cmd.codeUnits));
    }
  }

  void _onHoldStart(String cmd) {
    _sendCommand(cmd);
    _holdTimer = Timer.periodic(const Duration(milliseconds: 100), (_) {
      _sendCommand(cmd);
    });
  }

  void _onHoldEnd(String stopCmd) {
    _holdTimer?.cancel();
    _holdTimer = null;
    if (stopCmd.isNotEmpty) _sendCommand(stopCmd);
  }

  Future<void> _showDevicePicker() async {
    List<BluetoothDevice> devices =
        await FlutterBluetoothSerial.instance.getBondedDevices();
    if (!mounted) return;
    showDialog(
      context: context,
      builder: (ctx) => AlertDialog(
        backgroundColor: const Color(0xFF1A1A1A),
        title: const Text('Select Device', style: TextStyle(color: Colors.white)),
        content: SizedBox(
          width: double.maxFinite,
          child: ListView.builder(
            shrinkWrap: true,
            itemCount: devices.length,
            itemBuilder: (ctx, i) => ListTile(
              title: Text(devices[i].name ?? 'Unknown',
                  style: const TextStyle(color: Colors.white)),
              subtitle: Text(devices[i].address,
                  style: const TextStyle(color: Colors.grey)),
              onTap: () {
                Navigator.pop(ctx);
                _connect(devices[i]);
              },
            ),
          ),
        ),
      ),
    );
  }

  Future<void> _connect(BluetoothDevice device) async {
    try {
      setState(() => status = 'Connecting...');
      BluetoothConnection conn =
          await BluetoothConnection.toAddress(device.address);
      setState(() {
        connection = conn;
        isConnected = true;
        status = 'Connected: ${device.name}';
      });
    } catch (e) {
      setState(() => status = 'Failed to connect');
    }
  }

  @override
  void dispose() {
    _holdTimer?.cancel();
    connection?.dispose();
    super.dispose();
  }

  Widget _holdBtn(String text, Color bg, Color fg, String cmd,
      {String stopCmd = '', double height = 54}) {
    return Expanded(
      child: Listener(
        onPointerDown: (_) => _onHoldStart(cmd),
        onPointerUp: (_) => _onHoldEnd(stopCmd),
        onPointerCancel: (_) => _onHoldEnd(stopCmd),
        child: Container(
          height: height,
          decoration: BoxDecoration(
            color: bg,
            borderRadius: BorderRadius.circular(4),
          ),
          alignment: Alignment.center,
          child: Text(text,
              style: TextStyle(
                  color: fg, fontWeight: FontWeight.bold, fontSize: 15)),
        ),
      ),
    );
  }

  Widget _tapBtn(String text, Color bg, Color fg, String cmd,
      {double height = 54, double fontSize = 15}) {
    return Expanded(
      child: GestureDetector(
        onTap: () => _sendCommand(cmd),
        child: Container(
          height: height,
          decoration: BoxDecoration(
            color: bg,
            borderRadius: BorderRadius.circular(4),
          ),
          alignment: Alignment.center,
          child: Text(text,
              style: TextStyle(
                  color: fg,
                  fontWeight: FontWeight.bold,
                  fontSize: fontSize)),
        ),
      ),
    );
  }

  Widget _fullTapBtn(String text, Color bg, Color fg, String cmd,
      {double height = 54}) {
    return GestureDetector(
      onTap: () => _sendCommand(cmd),
      child: Container(
        width: double.infinity,
        height: height,
        decoration: BoxDecoration(
          color: bg,
          borderRadius: BorderRadius.circular(4),
        ),
        alignment: Alignment.center,
        child: Text(text,
            style: TextStyle(
                color: fg, fontWeight: FontWeight.bold, fontSize: 18)),
      ),
    );
  }

  Widget _label(String text) => Align(
        alignment: Alignment.centerLeft,
        child: Padding(
          padding: const EdgeInsets.only(bottom: 4),
          child: Text(text,
              style: const TextStyle(
                  color: Color(0xFF555555),
                  fontSize: 11,
                  fontFamily: 'monospace',
                  letterSpacing: 2)),
        ),
      );

  Widget _speedBtn(String label, String cmd, Color bg) {
    final bool selected = currentSpeed == cmd;
    return GestureDetector(
      onTap: () {
        setState(() => currentSpeed = cmd);
        _sendCommand(cmd);
      },
      child: Container(
        padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 8),
        decoration: BoxDecoration(
          color: selected ? bg : bg.withOpacity(0.35),
          borderRadius: BorderRadius.circular(4),
          border: selected
              ? Border.all(color: Colors.white, width: 2)
              : null,
        ),
        child: Text(label,
            style: TextStyle(
                color: selected ? Colors.white : Colors.white54,
                fontWeight: FontWeight.bold,
                fontSize: 13)),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(
        child: Column(
          children: [
            // TOP BAR
            Container(
              color: const Color(0xFF111111),
              padding:
                  const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
              child: Row(
                children: [
                  ElevatedButton(
                    style: ElevatedButton.styleFrom(
                      backgroundColor: isConnected
                          ? Colors.green
                          : const Color(0xFF00C853),
                      foregroundColor: Colors.black,
                      padding: const EdgeInsets.symmetric(
                          horizontal: 16, vertical: 8),
                    ),
                    onPressed: _showDevicePicker,
                    child: Text(
                        isConnected ? '● CONNECTED' : 'CONNECT',
                        style: const TextStyle(
                            fontWeight: FontWeight.bold, fontSize: 13)),
                  ),
                  const SizedBox(width: 10),
                  Text(status,
                      style: TextStyle(
                          color: isConnected
                              ? Colors.greenAccent
                              : Colors.redAccent,
                          fontSize: 12,
                          fontFamily: 'monospace')),
                  const Spacer(),
                  _speedBtn('START', '3', const Color(0xFF37474F)),
                  const SizedBox(width: 6),
                  _speedBtn('DRIVE', '5', const Color(0xFF1565C0)),
                  const SizedBox(width: 6),
                  _speedBtn('RACE', '8', const Color(0xFFE65100)),
                  const SizedBox(width: 6),
                  _speedBtn('MAX', '0', const Color(0xFFB71C1C)),
                ],
              ),
            ),

            // MAIN LAYOUT
            Expanded(
              child: Row(
                children: [
                  // DRIVE PANEL
                  Expanded(
                    child: Container(
                      color: const Color(0xFF111111),
                      margin: const EdgeInsets.all(8),
                      padding: const EdgeInsets.all(8),
                      child: SingleChildScrollView(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            _label('// DRIVE'),
                            _fullTapBtn('▲  FWD',
                                const Color(0xFF00C853), Colors.black, 'F', height: 46),
                            const SizedBox(height: 5),
                            Row(children: [
                              _holdBtn('◄', const Color(0xFF2979FF),
                                  Colors.white, 'L', stopCmd: 'S', height: 46),
                              const SizedBox(width: 5),
                              _tapBtn('STOP', const Color(0xFFFF1744),
                                  Colors.white, 'S', height: 46),
                              const SizedBox(width: 5),
                              _holdBtn('►', const Color(0xFF2979FF),
                                  Colors.white, 'R', stopCmd: 'S', height: 46),
                            ]),
                            const SizedBox(height: 5),
                            _label('CURVE  (hold to steer while moving)'),
                            Row(children: [
                              _holdBtn('↰ CURVE', const Color(0xFF1A237E),
                                  Colors.white, 'l', stopCmd: 'F', height: 46),
                              const SizedBox(width: 5),
                              _holdBtn('CURVE ↱', const Color(0xFF1A237E),
                                  Colors.white, 'r', stopCmd: 'F', height: 46),
                            ]),
                            const SizedBox(height: 5),
                            _fullTapBtn('▼  BWD',
                                const Color(0xFFFF6D00), Colors.white, 'B', height: 46),
                            const SizedBox(height: 8),
                            Row(children: [
                              _tapBtn('AUTO', const Color(0xFF37474F),
                                  Colors.white, 'A', height: 42),
                              const SizedBox(width: 5),
                              _tapBtn('MANUAL', const Color(0xFF263238),
                                  Colors.white, 'M', height: 42),
                            ]),
                          ],
                        ),
                      ),
                    ),
                  ),

                  Container(width: 1, color: const Color(0xFF222222)),

                  // ARM PANEL — SCROLLABLE
                  Expanded(
                    child: Container(
                      color: const Color(0xFF111111),
                      margin: const EdgeInsets.all(8),
                      padding: const EdgeInsets.all(8),
                      child: SingleChildScrollView(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            _label('// ARM'),
                            const SizedBox(height: 6),

                            _label('GRIPPER'),
                            Row(children: [
                              _tapBtn('OPEN', const Color(0xFF6200EA),
                                  Colors.white, 'O'),
                              const SizedBox(width: 6),
                              _tapBtn('CLOSE', const Color(0xFF9C27B0),
                                  Colors.white, 'C'),
                            ]),

                            const SizedBox(height: 10),
                            _label('ELBOW  (tap=nudge | hold=move)'),
                            Row(children: [
                              // 'u' = elbow nudge up
                              _holdBtn('ARM ▲', const Color(0xFF00838F),
                                  Colors.white, 'u'),
                              const SizedBox(width: 6),
                              // 'd' = elbow nudge down
                              _holdBtn('ARM ▼', const Color(0xFF006064),
                                  Colors.white, 'd'),
                            ]),

                            const SizedBox(height: 10),
                            _label('BASE  (tap=nudge | hold=move)'),
                            Row(children: [
                              _holdBtn('BASE ◄', const Color(0xFFE65100),
                                  Colors.white, 'q'),
                              const SizedBox(width: 6),
                              _holdBtn('BASE ►', const Color(0xFFBF360C),
                                  Colors.white, 'e'),
                            ]),

                            const SizedBox(height: 10),
                            _label('ARM PRESETS'),
                            Row(children: [
                              _tapBtn('ARM HOME', const Color(0xFF37474F),
                                  Colors.white, 'U', fontSize: 12),
                              const SizedBox(width: 6),
                              _tapBtn('ARM PICK', const Color(0xFF455A64),
                                  Colors.white, 'D', fontSize: 12),
                            ]),
                            const SizedBox(height: 10),
                            _label('SEQUENCES'),
                            Row(children: [
                              _tapBtn('🤏 PICK', const Color(0xFF00695C),
                                  Colors.white, 'P', fontSize: 13),
                              const SizedBox(width: 6),
                              _tapBtn('📦 PLACE', const Color(0xFF1565C0),
                                  Colors.white, 'X', fontSize: 13),
                            ]),
                            const SizedBox(height: 6),
                            Row(children: [
                              _tapBtn('↩ U-TURN', const Color(0xFF6A1B9A),
                                  Colors.white, 'T', fontSize: 13),
                              const SizedBox(width: 6),
                              _tapBtn('✕ CANCEL', const Color(0xFF7f0000),
                                  Colors.white, 'K', fontSize: 13),
                            ]),
                          ],
                        ),
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}
