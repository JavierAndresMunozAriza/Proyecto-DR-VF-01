"""Declaration of the five schematic sheets of DR-VF-01 (source: hardware/pinout.md).

Rules used here:
* IC pins: net name, NC, or Ser([...parts], net) for series parts on the pin.
* Everything else (decoupling, pull-ups, dividers) is a Branch between two nets.
* Power nets (GND, 3V3, VBAT, VBUS, VPANEL) are global power ports. Signals that cross sheets are ports.
* Resistor/capacitor values that are not in shared_lib use the nearest 0603 symbol and are listed in
  EDIT_VALUES so the report tells the user to copy a part and edit its value.
"""
from model import NC, Branch, Far, Par, Part, Ser, Sheet, Tp

RES_LIB = {
    "0": "Res_0R_0603_1_10W", "100": "Res_100R_0603_1_10W", "10k": "Res_10k_0603_1_10W",
    "120": "Res_120R_0603_1_10W", "150": "Res_150R_0603_1_10W", "1k": "Res_1k_0603_1_10W",
    "330": "Res_330R_0603_1_10W", "390k": "Res_390k_0603_1_10W", "4.7k": "Res_4k7_0603_1_10W",
    "3k3": "Res_3k3_0603_1_10W",
}
CAP_LIB = {
    "100nF": "Cap_0.1uF_0603_50V", "1uF": "Cap_1uF_0603_50V", "470nF": "Cap_470nF_0603_50V",
    "33pF": "Cap_33pF_0603_50V", "10uF/10V": "Cap_10uF_0603_10V", "10uF/25V": "Cap_10uF_0805_25V_",
    "22uF/16V": "Cap_22uF_0805_16V", "100uF/1206": "Cap_100uF_1206_DNP", "100uF/35V": "Cap_100uF_ALU_TH_35V",
}
EDIT_VALUES = []  # (value, what to copy) collected while building


def R(value, ref=None, **kw):
    lib = RES_LIB.get(value)
    if lib is None:
        lib = RES_LIB["1k"]
        EDIT_VALUES.append(("R", value))
    return Part(lib, value if value.endswith(("k", "M")) or "." in value or value == "0" else value, ref, **kw)


def C(value, ref=None, **kw):
    lib = CAP_LIB[value]
    return Part(lib, value.split("/")[0] + (" " + value.split("/")[1] if "/" in value and value != "100uF/1206" else ""),
                ref, **kw)


# ------------------------------------------------------------------ sheet 01 Potencia
def sheet1():
    s = Sheet(1, "Potencia y carga", "A3", ports={"USB_DP": "io", "USB_DM": "io", "CHRG_STAT": "out", "CHG_INHIBIT": "in"})
    s.titles = [(60, 1120, "1A  Entrada USB-C y proteccion ESD"), (880, 1120, "1B  Cargador USB (TP4056)"),
                (60, 790, "1C  Cargador solar CN3791: controlador"), (880, 790, "1D  CN3791: etapa de potencia"),
                (880, 600, "1E  Conectores y regulador 3.3 V")]
    s.regions = [("1A USB-C y proteccion ESD", 40, 820, 800, 300), ("1B TP4056", 880, 820, 760, 300),
                 ("1C CN3791, controlador", 400, 270, 520, 510), ("1D CN3791, etapa de potencia", 880, 660, 760, 130),
                 ("1E Conectores y regulador", 880, 300, 760, 340)]
    # 1A USB-C
    s.add(Part("USB_TYPEC_16P", "USB C", ref="J1", at=(100, 960), pins={
        "A5": "CC1", "B5": "CC2", "A6": "USBC_DP", "B6": "USBC_DP", "A7": "USBC_DN", "B7": "USBC_DN",
        "A8": NC("SBU1"), "B8": NC("SBU2"),
        "A4/B9": "VBUS", "B4/A9": "VBUS", "A1/B12": "GND", "B1/A12": "GND", "0": "GND"}))
    s.branches += [Branch("CC1", [R("5.1k", "R2")], "GND", (250, 985)), Branch("CC2", [R("5.1k", "R1")], "GND", (320, 985))]
    s.add(Part("TVS_5.25V_2CH_SOT_23_6", "USBLC6-2SC6", ref="U1", at=(520, 1000), pins={
        "1": "USBC_DP", "3": "USBC_DN", "2": "GND", "5": "VBUS", "6": "USBLC_DP", "4": "USBLC_DN"}))
    s.branches += [Branch("USBLC_DP", [R("22", "R3")], "USB_DP", (700, 1010), "h"),
                   Branch("USBLC_DN", [R("22", "R4")], "USB_DM", (700, 960), "h"),
                   Branch("VBUS", [Part(CAP_LIB["10uF/25V"], "10uF 25V X7R", "C1")], "GND", (660, 1090)),
                   Branch("USB_DP", [C("33pF", "C31", dnp=True)], "GND", (790, 1090)),
                   Branch("USB_DM", [C("33pF", "C32", dnp=True)], "GND", (850, 1090))]
    # 1B TP4056
    s.add(Part("IC_BAT_CHG_4.2V_1A", "TP4056", ref="U2", at=(1000, 960), pins={
        "4": "VBUS", "8": "VBUS", "1": "GND", "3": "GND", "9": "GND", "2": "PROG", "5": "VBAT",
        "6": "GND", "7": "GND"}))
    s.branches += [Branch("PROG", [R("1.8k", "R5"), R("100", "R6")], "GND", (1200, 960)),
                   Branch("VBAT", [C("10uF/25V", "C2")], "GND", (1320, 1010))]
    # 1C CN3791 controller
    s.add(Part("CN3791", "CN3791", ref="U3", at=(700, 560), pins={
        "1": "VG", "9": "VPANEL", "3": "CHRG_STAT", "4": Far("GND", 90), "6": "MPPT",
        "10": "DRV", "8": "CSP", "7": "VBAT", "5": "COM", "2": "GND"}))
    s.branches += [
        Branch("VG", [C("100nF", "C8")], "VPANEL", (590, 700)),
        Branch("3V3", [R("100k", "R11")], "CHRG_STAT", (470, 700)),
        Branch("VPANEL", [R("390k", "R8"), R("10k", "R9")], "MPPT", (540, 500)),
        Branch("MPPT", [R("100k", "R10")], "GND", (620, 500)),
        Branch("COM", [R("120", "R7"), C("470nF", "C9"), C("470nF", "C10")], "GND", (830, 580)),
        Branch("CHG_INHIBIT", [R("100k", "R32")], "GND", (820, 390)),
    ]
    s.add(Part("Q_NFET_20V_3.2A_SOT23", "N-FET", ref="Q3", at=(720, 330), pins={
        "1": "CHG_INHIBIT", "2": "GND", "3": "MPPT"}))
    # 1D power stage
    s.add(Part("Q_PFET_30V_3A_SOT23", "AP2301", ref="Q1", at=(960, 700), pins={"1": "DRV", "2": "VPANEL", "3": "M1_D"}))
    s.branches += [
        Branch("M1_D", [Part("D_Schottky_40V_1A_SOD_123F", "PSBD1DF40V1H", "D1", first="A")], "SW", (1060, 730)),
        Branch("SW", [Part("D_Schottky_40V_1A_SOD_123F", "PSBD1DF40V1H", "D2", first="K")], "GND", (1150, 730)),
        Branch("SW", [Part("Inductor_47uH_0.71A_5.8_5.2mm", "47uH", "L1")], "CSP", (1260, 720), "h"),
        Branch("CSP", [R("0.47", "R12")], "VBAT", (1450, 720), "h"),
    ]
    # 1E connectors, bulk capacitors, regulator
    s.add(Part("Con_2x1_TH_JST_2MM", "Panel 6 V", ref="J3", at=(1000, 520), pins={"1": Far("GND", 40), "2": "VPANEL"}))
    s.add(Part("Con_2x1_TH_JST_2MM", "Bateria", ref="J2", at=(1000, 420), pins={"1": Far("GND", 40), "2": "BATT_J2"}))
    s.add(Part("Q_PFET_30V_3A_SOT23", "AP2301 batería inversa", ref="Q2", at=(1130, 350), pins={
        "1": "GND", "2": "VBAT", "3": "BATT_J2"}))
    s.tps += [Tp("VBAT", (1230, 380), "down", ref="TP8")]
    s.branches += [
        Branch("VPANEL", [C("100uF/35V", "C3", first="+")], "GND", (1150, 610)),
        Branch("VPANEL", [C("10uF/25V", "C4")], "GND", (1230, 610)),
        Branch("VPANEL", [C("470nF", "C5")], "GND", (1310, 610)),
        Branch("VBAT", [C("10uF/25V", "C6")], "GND", (1150, 480)),
        Branch("VBAT", [C("10uF/25V", "C7")], "GND", (1230, 480)),
    ]
    s.add(Part("TLV75733PDBVR", "TLV75733P", ref="U4", at=(1480, 480), pins={
        "1": "VBAT", "3": "VBAT", "2": "GND", "4": NC("sin conexion interna"), "5": "3V3_LDO"}))
    s.branches += [Branch("VBAT", [C("10uF/25V", "C11")], "GND", (1340, 480)),
                   Branch("3V3_LDO", [C("1uF", "C12")], "GND", (1590, 400)),
                   Branch("3V3_LDO", [R("0", "R33")], "3V3", (1500, 330), "h")]
    s.texts = [(880, 255, "Q2 (AP2301): drenador a J2, fuente a VBAT, puerta a GND. Protege contra bateria conectada al reves.", 8),
               (880, 241, "Q3 (N-FET): corta la carga solar llevando MPPT a GND; CHG_INHIBIT viene del GPIO21 (retencion RTC en sueno).", 8),
               (880, 227, "R33 (0 ohm): puente para medir la corriente de todo el riel de 3.3 V. TP8: pad de prueba en VBAT.", 8),
               (880, 213, "STDBY y CHRG del TP4056 a GND (el fabricante lo pide); C31 y C32 sin montar (huellas para Espressif).", 8)]
    return s


# ------------------------------------------------------------------ sheet 02 MCU
def sheet2():
    ports = {"USB_DP": "io", "USB_DM": "io", "CHRG_STAT": "in", "IMU_INT1": "in", "I2C_SDA": "io", "I2C_SCL": "io",
             "LORA_RESET": "out", "LORA_DIO1": "in", "LORA_NSS": "out", "LORA_MOSI": "out", "LORA_SCK": "out",
             "LORA_MISO": "in", "LORA_BUSY": "in", "LORA_RXEN": "out", "LORA_TXEN": "out",
             "GNSS_TX": "out", "GNSS_RX": "in", "CHG_INHIBIT": "out", "GNSS_RESET": "out",
             "OPTO_CTRL": "out", "BUZZER_PWM": "out", "BUTTON": "in", "LED_R": "out", "LED_G": "out", "LED_B": "out"}
    s = Sheet(2, "Microcontrolador ESP32-S3", "A3", ports=ports)
    s.titles = [(300, 1120, "2A  Modulo ESP32-S3-WROOM-1U-N16R8"), (300, 960, "2B  Arranque: EN, BOOT y UART0"),
                (960, 960, "2C  Alimentacion y cristal de 32.768 kHz")]
    s.regions = [("2A Modulo y puertos", 560, 420, 760, 560), ("2B EN, BOOT y UART0", 280, 560, 460, 560),
                 ("2C Alimentacion y cristal", 940, 560, 560, 560)]
    s.add(Part("ESP32-S3-WROOM-1U", "ESP32-S3-WROOM-1U-N16R8", at=(820, 660), pins={
        "1": "GND", "40": "GND", "41_1": "GND", "41_2": "GND", "41_3": "GND", "41_4": "GND", "41_5": "GND",
        "41_6": "GND", "41_7": "GND", "41_8": "GND", "41_9": "GND",
        "2": "3V3", "3": "EN",
        "4": "IMU_INT1", "5": "LORA_RESET", "6": "LORA_DIO1", "7": "BUTTON",
        "8": "XTAL_32K_P", "9": "XTAL_32K_N", "10": "GNSS_TX", "11": "GNSS_RX", "12": "I2C_SDA",
        "13": "USB_DM", "14": "USB_DP", "15": NC("IO3 strapping JTAG"), "16": NC("IO46 strapping"),
        "17": "I2C_SCL", "18": "LORA_NSS", "19": "LORA_MOSI", "20": "LORA_SCK", "21": "LORA_MISO",
        "22": "LORA_BUSY", "23": "CHG_INHIBIT", "24": "LORA_TXEN", "25": "GNSS_RESET",
        "26": NC("IO45 strapping"), "27": "BOOT", "28": NC("PSRAM"), "29": NC("PSRAM"), "30": NC("PSRAM"),
        "31": "LED_R", "32": "LED_G", "33": "LED_B", "34": "CHRG_STAT", "35": "LORA_RXEN",
        "36": "UART0_RX", "37": "UART0_TXD", "38": "BUZZER_PWM", "39": "OPTO_CTRL"}))
    # 2B EN, BOOT, UART0
    s.branches += [
        Branch("3V3", [R("10k")], "EN", (350, 900)),
        Branch("EN", [C("1uF")], "GND", (430, 900)),
        Branch("3V3", [R("10k")], "BOOT", (350, 760)),
        Branch("UART0_TXD", [R("330"), R("150")], "UART0_TX", (440, 640), "h"),
        Branch("EN", [Part("Sw_PUSH_4.5x4.5x5.0xmm", "TSC015A05026A", first="N")], "GND", (620, 900)),
        Branch("BOOT", [Part("Sw_PUSH_4.5x4.5x5.0xmm", "TSC015A05026A", first="N")], "GND", (620, 760)),
    ]
    s.tps += [Tp("EN", (510, 890)), Tp("BOOT", (430, 750)), Tp("UART0_RX", (350, 650)), Tp("UART0_TX", (640, 650)),
              Tp("GND", (510, 740), "up")]
    # 2C power and crystal
    s.branches += [
        Branch("3V3", [C("22uF/16V")], "GND", (1000, 900)),
        Branch("3V3", [C("100nF")], "GND", (1080, 900)),
        Branch("3V3", [C("10uF/10V")], "GND", (1160, 900)),
    ]
    s.tps += [Tp("3V3", (1240, 890))]
    s.add(Part("ECS-.327-12.5-34B-TR", "32.768 kHz", at=(1100, 740), pins={"1": "XTAL_32K_P", "2": "XTAL_32K_N"}))
    s.branches += [
        Branch("XTAL_32K_P", [C("33pF"), C("33pF")], "GND", (1030, 660)),
        Branch("XTAL_32K_N", [C("33pF"), C("33pF")], "GND", (1170, 660)),
    ]
    s.texts = [(300, 340, "IO3, IO45, IO46 (pines 15, 26, 16) y IO35 a IO37 (pines 28 a 30) quedan sin conectar.", 8),
               (300, 326, "El pad central (pin 41) son 9 pines GND del simbolo: todos a GND.", 8),
               (300, 312, "UART0_TX: 330 + 150 = 480 ohm (el valor de 499 ohm de la guia de Espressif no esta en el stock).", 8),
               (300, 298, "Botones EN y BOOT en paralelo con TP1 y TP2: mantener BOOT, pulsar y soltar EN, soltar BOOT = modo descarga.", 8),
               (300, 284, "IO21 = CHG_INHIBIT (corte de carga solar, GPIO RTC con retencion en sueno); IO48 = GNSS_RESET (antes reserva del modem de la Fase 2).", 8)]
    return s


# ------------------------------------------------------------------ sheet 03 RF
def sheet3():
    ports = {"GNSS_TX": "in", "GNSS_RX": "out", "GNSS_RESET": "in", "LORA_RESET": "in", "LORA_DIO1": "out",
             "LORA_NSS": "in", "LORA_MOSI": "in", "LORA_SCK": "in", "LORA_MISO": "out", "LORA_BUSY": "out",
             "LORA_RXEN": "in", "LORA_TXEN": "in"}
    s = Sheet(3, "Radio: GNSS y LoRa", "A3", ports=ports)
    s.titles = [(60, 1120, "3A  GNSS u-blox MAX-M10S"), (760, 1120, "3B  Modulo LoRa DX-LR30-900M22S")]
    s.regions = [("3A GNSS", 40, 500, 580, 640), ("3B LoRa", 620, 500, 1000, 640)]
    s.add(Part("MAX-M10S-00B", "MAX-M10S-00B", at=(420, 760), pins={
        "8": "3V3_GNSS", "7": "3V3_GNSS", "14": NC("VCC_RF"), "6": NC("V_BCKP"), "4": NC("TIMEPULSE"), "13": NC("LNA_EN"),
        "2": "GNSS_RX", "3": "GNSS_TX", "16": NC("SDA"), "17": NC("SCL"),
        "1": "GND", "10": "GND", "12": "GND",
        "9": "GNSS_RESET_N", "11": "GNSS_RF", "5": NC("EXTINT"), "18": "GNSS_SAFEBOOT", "15": NC("VIO_SEL")}))
    s.tps += [Tp("GNSS_SAFEBOOT", (160, 650)), Tp("GNSS_TX", (520, 600), ref="TP9"), Tp("GNSS_RX", (580, 600), ref="TP10")]
    s.add(Part("U.FL-R-SMT-1(10)", "u.FL GNSS", at=(110, 760), pins={"1": "GND", "3": "GND", "2": "GNSS_RF"}))
    s.branches += [Branch("3V3_GNSS", [C("100nF")], "GND", (300, 950)), Branch("3V3_GNSS", [C("10uF/10V")], "GND", (380, 950)),
                   Branch("3V3", [R("0", "R35")], "3V3_GNSS", (560, 1040), "h"),
                   Branch("GNSS_RESET", [R("0", "R34")], "GNSS_RESET_N", (180, 540), "h")]
    s.add(Part("DX-LR30-900M22S", "DX-LR30-900M22S", at=(940, 900), pins={
        "1": "GND", "2": "GND", "3": "GND", "4": "GND", "5": NC("DIO3"), "6": Far("LORA_RXEN_P", 8), "7": "LORA_TXEN_P",
        "8": "LORA_DIO2", "9": "3V3_LORA", "10": "GND", "11": "GND",
        "22": Far("GND", 70), "21": "LORA_ANT", "20": Far("GND", 70), "19": "LORA_NSS", "18": Far("LORA_SCK_M", 90),
        "17": "LORA_MOSI", "16": "LORA_MISO", "15": "LORA_RESET", "14": "LORA_BUSY", "13": "LORA_DIO1", "12": "GND"}))
    s.branches += [
        Branch("LORA_RXEN_P", [R("0")], "LORA_RXEN", (650, 880), "h"),
        Branch("LORA_TXEN_P", [R("0")], "LORA_TXEN", (650, 820), "h"),
        Branch("LORA_DIO2", [R("0", dnp=True)], "LORA_TXEN_P", (650, 760), "h"),
        Branch("3V3", [R("0", "R36")], "3V3_LORA", (650, 700), "h"),
        Branch("3V3", [R("100k", "R37")], "LORA_NSS", (1250, 790)),
        Branch("LORA_RXEN_P", [R("100k", "R38")], "GND", (1110, 720)),
        Branch("LORA_TXEN_P", [R("100k", "R39")], "GND", (1190, 720)),
        Branch("LORA_SCK_M", [R("0", "R40")], "LORA_SCK", (1290, 840), "h"),
        Branch("LORA_SCK_M", [C("33pF", "C33", dnp=True)], "GND", (1380, 790)),
        Branch("3V3_LORA", [C("10uF/10V")], "GND", (780, 720)),
        Branch("3V3_LORA", [C("100nF")], "GND", (860, 720)),
        Branch("3V3_LORA", [C("22uF/16V")], "GND", (940, 720)),
        Branch("3V3_LORA", [C("100uF/1206", dnp=True)], "GND", (1020, 720)),
    ]
    s.add(Part("U.FL-R-SMT-1(10)", "u.FL LoRa", at=(1330, 900), pins={"1": "GND", "3": "GND", "2": "LORA_ANT"}))
    s.texts = [(60, 440, "RX y TX del modulo son entradas del conmutador de antena: 0 ohm en cada una (pinout.md).", 8),
               (60, 426, "El 0 ohm entre DIO2 y TX queda SIN MONTAR (DNP); solo se monta si DIO2 gobierna el conmutador.", 8),
               (60, 412, "Antena del LoRa: pad ANT (pin 21) hacia el u.FL; confirmar con el vendedor antes del layout.", 8),
               (60, 398, "El 1206 de 100 uF queda sin montar: solo si el osciloscopio muestra caida del riel en el TX.", 8),
               (60, 384, "R35 y R36 (0 ohm): puentes para medir la corriente del GNSS y del LoRa. R34: RESET_N del GNSS a GPIO48; se puede quitar.", 8),
               (60, 370, "R37: pull-up de LORA_NSS. R38 y R39: pull-downs de RXEN y TXEN (lado del modulo). R40, C33: SCK con 0 ohm y condensador sin montar.", 8),
               (60, 356, "EXTINT del GNSS sin conectar (opcional; abierto queda en alto). TP9 y TP10: pads de prueba del UART del GNSS.", 8)]
    return s


# ------------------------------------------------------------------ sheet 04 Sensores
def sheet4():
    s = Sheet(4, "Sensores: IMU y medidor de bateria", "A4", ports={
        "IMU_INT1": "out", "I2C_SDA": "io", "I2C_SCL": "io"})
    s.titles = [(60, 780, "4A  IMU LSM6DS3TR-C (I2C, direccion 0x6A)"), (560, 780, "4B  Medidor de bateria MAX17048 (I2C, 0x36)")]
    s.regions = [("4A IMU", 40, 400, 520, 400), ("4B Medidor", 520, 400, 600, 400)]
    s.add(Part("LSM6DS3TR-C", "LSM6DS3TR-C", at=(250, 640), pins={
        "1": "GND", "2": "GND", "3": "GND", "4": "IMU_INT1", "5": "3V3", "6": "GND", "7": "GND",
        "14": "I2C_SDA", "13": "I2C_SCL", "12": "3V3", "11": NC("NC soldado"), "10": NC("NC soldado"),
        "9": NC("INT2"), "8": "3V3"}))
    s.branches += [Branch("3V3", [C("100nF")], "GND", (160, 530)), Branch("3V3", [C("100nF")], "GND", (240, 530))]
    s.add(Part("MAX17048G+T10", "MAX17048G+T10", at=(800, 640), pins={
        "6": "GND", "7": "I2C_SCL", "8": "I2C_SDA", "1": "GND", "2": "VBAT", "3": "VBAT", "9": "GND",
        "5": NC("ALRT"), "4": "GND"}))
    s.branches += [Branch("VBAT", [C("100nF")], "GND", (960, 520))]
    s.branches += [Branch("3V3", [R("3k3")], "I2C_SDA", (560, 470)), Branch("3V3", [R("3k3")], "I2C_SCL", (640, 470))]
    s.tps += [Tp("IMU_INT1", (330, 520), ref="TP11")]
    s.texts = [(60, 340, "Pull-ups I2C de 3.3 kohm compartidos por el IMU y el medidor (cumplen 300 ns a 400 kHz con 80 pF; con 4.7 kohm daban 319 ns).", 8),
               (60, 326, "El MAX17048 toma VDD y CELL del positivo de la bateria (VBAT); el bus I2C va a 3.3 V.", 8)]
    return s


# ------------------------------------------------------------------ sheet 05 Interfaz
def sheet5():
    s = Sheet(5, "Interfaz: optoacoplador, LED, zumbador y boton", "A4", ports={
        "OPTO_CTRL": "in", "BUZZER_PWM": "in", "BUTTON": "out", "LED_R": "in", "LED_G": "in", "LED_B": "in"})
    s.titles = [(60, 780, "5A  Optoacoplador de salida (RF-03)"), (560, 780, "5B  LED RGB, zumbador y boton")]
    s.regions = [("5A Optoacoplador", 40, 440, 500, 360), ("5B LED RGB, zumbador y boton", 520, 160, 620, 640)]
    s.add(Part("VO617A-1X017T", "VO617A-1X017T", at=(300, 640), pins={
        "1": "OPTO_A", "2": "GND", "3": "GND", "4": "OPTO_C"}))
    s.branches += [
        Branch("OPTO_CTRL", [R("330")], "OPTO_A", (130, 650), "h"),
        Branch("OPTO_CTRL", [R("100k")], "GND", (130, 560)),
        Branch("3V3", [R("1k"), Part("LED_0805_RED", "LED rojo", first="A")], "OPTO_C", (460, 700)),
    ]
    s.add(Part("LED_RGB_3535_SMD", "RS-3535MWAM", at=(700, 640), pins={
        "4": "3V3", "5": "3V3", "6": "3V3", "3": "LED_R_K", "2": "LED_G_K", "1": "LED_B_K"}))
    s.branches += [
        Branch("LED_R_K", [Par([R("1k"), R("3k")])], "LED_R", (900, 680), "h"),
        Branch("LED_G_K", [R("470")], "LED_G", (900, 610), "h"),
        Branch("LED_B_K", [R("330")], "LED_B", (900, 560), "h"),
    ]
    s.add(Part("PS1240P02BT", "PS1240P02BT", at=(820, 420), pins={"1": "BUZZ_A", "2": "GND"}))
    s.branches += [Branch("BUZZER_PWM", [R("100")], "BUZZ_A", (620, 420), "h")]
    s.add(Part("Sw_PUSH_4.5x4.5x5.0xmm", "TSC015A05026A", ref="SW1", at=(700, 260), pins={"1": "GND", "2": "BTN_N"}))
    s.branches += [
        Branch("3V3", [R("10k")], "BTN_N", (860, 330)),
        Branch("BTN_N", [R("1k")], "BUTTON", (960, 260), "h"),
        Branch("BUTTON", [C("100nF")], "GND", (1090, 330)),
    ]
    s.texts = [(60, 440, "LED rojo del optoacoplador: anodo a 3V3 por 1 kohm, catodo al colector (pin 4).", 8),
               (60, 426, "Pull-down de 100 kohm en OPTO_CTRL, antes de la resistencia de 330 ohm (arranca apagado).", 8),
               (60, 412, "LED RGB: anodos a 3V3 y catodos al GPIO por la resistencia; activo en bajo. 750 ohm = 1k // 3k.", 8),
               (60, 398, "Zumbador de la libreria: PS1240P02BT (prueba). pinout.md describe el Murata: mismas conexiones.", 8)]
    return s


def all_sheets():
    return [sheet1(), sheet2(), sheet3(), sheet4(), sheet5()]
