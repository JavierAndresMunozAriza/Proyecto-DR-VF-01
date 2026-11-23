# PCB outputs

Fabrication and documentation outputs of the DR-VF-01 board, exported from Altium on 2026-11-18. The Altium sources (`.PrjPcb`, `.SchDoc`, `.PcbDoc`) are not in the repository.

| File | Content |
| :--- | :--- |
| `DR-VF-01_schematics.pdf` | The 5 schematic sheets: power and charge, ESP32-S3, GNSS and LoRa radio, IMU and fuel gauge, optocoupler, LED, buzzer and button |
| `DR-VF-01_draftsman.pdf` | Assembly drawing with designators |
| `DR-VF-01_BOM.csv` | Bill of materials exported from Altium (54 lines); `../BOM.md` explains the choices |
| `DR-VF-01_gerbers.zip` | Gerber files (Top and Bottom layers, solder mask, paste, overlay, board shape) and NC drill files |

Design rule check of 2026-11-15: 0 violations in all rules, including un-routed nets and short circuits. The board is **designed, not fabricated**: nothing here has been measured on hardware.

**The BOM CSV documents the design; do not use it to order parts.** The design review (`docs/design-reviews/bom.md`, findings BOM-02 and BOM-03) found wrong manufacturer and supplier parameters on 25 placed parts and no marking of the parts that are not fitted.
