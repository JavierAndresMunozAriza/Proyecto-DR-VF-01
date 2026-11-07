# Mensaje del docente en el classroom (16 de octubre de 2026)

Transcrito el 2026-11-05 de la captura que mandó el autor. **Es la lista de lo que el docente tiene físicamente fuera de su biblioteca**, además de todo lo que trae `shared_lib.IntLib` (ver `hardware/COMPRAS.md` sección 1 y `hardware/AUDITORIA_STOCK_DOCENTE.md`).

## Texto original

> Adjunto librería con componentes que pueden usar, y la guía de DCDCs.
>
> Componentes adicionales que tengo y no aparecen en librería:
>
> - Battery Charging IC Lithium-ion/Polymer ESOP-8 (TP4056)
> - Amplifier 50mA 4 300uV 6.5V/us 103dB 0.5pA 538uA 10MHz Rail-t (TLV9064IDR)
> - IMU (LSM6DS3TR-C)
> - Low-Power Digital Temperature Sensor With SMBus and Two-Wire Serial Interface (TMP102AIDRLR)
> - ±18K ESD Protection, Full Fail-Safe RS-485 Transceiver (TP8485E-SR)
> - High-side measurement shunt monitor (INA139NA/3K)
> - POT 10kΩ 150mW SMD-3P, 3.8x3.6mm (TC33X-2-103E)
> - 3A, Ultra-Low Dropout Voltage Regulator (DS8570F8)

Nota de transcripción: en la captura, la línea del IMU y la del sensor de temperatura salen pegadas ("IMU (LSM6DS3TR-C)Low-Power Digital Temperature Sensor ... (TMP102AIDRLR)"). Se leen como dos piezas distintas, porque el LSM6DS3TR-C es un IMU y el TMP102 es un sensor de temperatura.

## Cruce con el diseño y con la biblioteca (2026-11-05)

| Pieza del classroom | ¿Está en `shared_lib.IntLib`? | ¿La usa el diseño hoy? | Qué corresponde |
|---------------------|-------------------------------|------------------------|-----------------|
| TP4056 (ESOP-8) | **Sí**: `IC_BAT_CHG_4.2V_1A`, YONGYUTAI, LCSC C20199357, huella `SOIC_8_3.9mm` sin pad térmico. El mensaje dice que no aparece; la biblioteca la trae | Sí (carga por USB-C) | No comprar. No bajar una librería nueva: revisar si la huella `SOIC_8_3.9mm_EP` que ya trae la biblioteca sirve para el ESOP-8. **Revisado el 2026-11-06:** sirve, con el pad 9 a GND (`PLAN_MANANA.md`, paso 19) |
| LSM6DS3TR-C | No | Sí (IMU, RF-06) | No comprar. Su librería se bajó (Ultra Librarian de ST) y está verificada |
| DS8570F8 (3 A, caída ultrabaja) | No | No. El diseño compra el TLV75733PDBVR | Posible reemplazo del LDO. **Sin decidir**: falta su datasheet (DSTECH, LCSC C5884245) para ver la corriente quiescente y la alimentación auxiliar que exige |
| TMP102AIDRLR (temperatura, I2C) | No | No | Disponible gratis. Oportunidad de diseño, no requisito (ver auditoría, sección 5) |
| TLV9064IDR (amplificador operacional) | No | No | Sin uso |
| TP8485E-SR (RS-485) | No | No | Sin uso |
| INA139NA/3K (monitor de corriente en el lado alto) | No | No | Sin uso hoy; serviría para medir la corriente de carga solar |
| TC33X-2-103E (potenciómetro de 10 kΩ) | No | No | Sin uso hoy; podría afinar el divisor MPPT del CN3791 |

Regla del autor: de lo que el docente tiene en el classroom y no en la biblioteca, **se baja la librería pero no se compra**. De lo que ya está en la biblioteca, no se baja ni se compra nada.

Pendiente: las cantidades que tiene el docente de cada pieza no están confirmadas.
