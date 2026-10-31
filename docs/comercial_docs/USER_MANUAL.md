# Manual de Usuario - Collar de Geocercamiento DR-VF-01

## Bienvenido

¡Gracias por elegir el Collar de Geocercamiento DR-VF-01! Este dispositivo avanzado le permitirá monitorear y contener eficientemente su ganado en grandes extensiones de pastoreo.

Este manual le guiará a través de la instalación, configuración y uso diario del collar. Por favor, léalo completamente antes de usar el dispositivo.

## Contenido del Embalaje

Verifique que su kit incluya:
- 1x Collar DR-VF-01 completamente ensamblado
- 1x Correa de ajuste universal
- 1x Cable de carga USB-C
- 1x Guía de inicio rápido
- Acceso al portal web de configuración (credenciales separadas)

## Instalación Inicial

### Paso 1: Carga Inicial
Antes de usar el collar por primera vez:

1. Retire la tapa ciega del puerto USB-C (tapas ciegas USB-C: POR DECIDIR; sellado pendiente)
2. Conecte el cable USB-C al puerto de carga del collar
3. Conecte el otro extremo a un cargador USB de 5V (el collar limita la carga a 500mA, no se necesita un cargador de 1A)
4. Espere el tiempo de carga: no menos de 2.4 horas con una batería de 1200mAh o 3 horas con una de 1500mAh (capacidad POR DECIDIR), antes de la fase final de tensión constante. La indicación por LED de carga completa por USB-C está POR DECIDIR (hardware no definido)
5. Desconecte el cable de carga y vuelva a colocar la tapa del puerto USB-C hasta que cierre por completo

### Paso 2: Ajuste de la Correa
1. Coloque el collar alrededor del cuello del animal
2. Ajuste la correa asegurando que:
   - Quepa una mano entre el collar y el cuello
   - No esté tan apretado como para causar incomodidad
   - La carcasa electrónica quede centrada en la parte superior del cuello
3. Asegure la hebilla de seguridad y corte el exceso de correa

### Paso 3: Activación del Dispositivo
1. Presione y mantenga el botón lateral por 5 segundos
2. Espere a que el LED parpadee en azul (modo de configuración)
3. El dispositivo está ahora listo para configuración inicial

## Configuración Inicial

### Acceso al Portal de Configuración
1. En su teléfono inteligente o computadora, active el Wi-Fi
2. Busque y conéctese a la red "DR-VF-01-CONFIG"
3. Abra un navegador web y visite http://192.168.4.1
4. Ingrese las credenciales proporcionadas (usuario: admin, contraseña: geofence2024)

### Configuración de Geocerca
1. En la interfaz web, seleccione "Geocerca" en el menú principal
2. Utilice el mapa interactivo para:
   - Centrar en la ubicación de su pastoreo
   - Dibujar el perímetro de la zona permitida tocando puntos en el mapa
   - Ajustar vértices arrastrándolos
3. Configure las zonas de exclusión si es necesario (función planificada; la versión actual admite un único polígono)
4. Guarde la configuración

### Configuración de Sensibilidad
1. Vaya a "Configuración" > "Movimiento"
2. Ajuste el umbral de detección:
   - Bajo (0.1g): Para animales tranquilos o ancianos
   - Medio (0.5g): Configuración recomendada
   - Alto (1.0g): Para animales muy activos
3. Configure la duración de activación (1-10 segundos)

### Configuración de Comunicación
1. Seleccione "Comunicación" en el menú
2. Configure las opciones de LoRa:
   - Frecuencia: 915MHz (predeterminado)
   - Potencia: +22dBm (máximo del transceptor SX1262)
   - Intervalo de reporte: 1-60 minutos
3. Si dispone de gateway LoRa, ingrese sus credenciales

## Uso Diario

### Indicadores LED
- **Verde fijo**: Cargando (indicación de carga por USB-C POR DECIDIR; hardware no definido)
- **Verde parpadeante**: Carga completa (indicación de carga por USB-C POR DECIDIR; hardware no definido)
- **Azul parpadeante**: Modo configuración
- **Rojo fijo**: Batería baja
- **Rojo parpadeante**: Alerta de geocerca activa
- **Amarillo parpadeante**: Error del sistema

### Monitoreo Remoto
1. Utilice la aplicación móvil (disponible próximamente) o
2. Configure su gateway LoRa para recibir datos
3. Acceda al dashboard web para visualizar:
   - Posición actual de cada animal
   - Historial de movimientos
   - Estado de batería
   - Alertas y eventos

### Mantenimiento Regular
1. **Limpieza**: Limpie la carcasa con un paño húmedo cada semana
2. **Inspección visual**: Verifique daños en correa y carcasa
3. **Carga**: Cargue cuando el LED se vuelva rojo
4. **Actualizaciones**: Verifique actualizaciones mensualmente

## Solución de Problemas

### Problema: El collar no se enciende
**Soluciones**:
1. Verifique que la batería esté cargada
2. Presione y mantenga el botón lateral por 10 segundos
3. Si persiste, realice un reinicio de fábrica (ver abajo)

### Problema: No puedo conectarme al Wi-Fi de configuración
**Soluciones**:
1. Asegúrese de estar a menos de 2 metros del collar
2. Reinicie el collar (presione botón 10 segundos)
3. Reinicie su dispositivo móvil
4. Verifique que no haya muchas redes Wi-Fi congestionando el espectro

### Problema: El collar no detecta movimiento
**Soluciones**:
1. Verifique la sensibilidad en la configuración web
2. Asegúrese de que el collar no esté demasiado flojo
3. Reinicie el sensor IMU desde la interfaz web

### Problema: Las alertas de geocerca son falsas
**Soluciones**:
1. Verifique la precisión del GPS en su área
2. Ajuste la geocerca para incluir un margen de seguridad
3. Considere interferencias magnéticas en la zona

## Reinicio de Fábrica

Para restaurar la configuración de fábrica:
1. Presione y mantenga el botón lateral por 30 segundos
2. Espere a que el LED parpadee rápidamente en rojo y azul
3. El dispositivo se reiniciará con configuración predeterminada
4. Vuelva a realizar la configuración inicial

## Especificaciones Técnicas

### Dimensiones y Peso
- Dimensiones: 120mm x 80mm x 25mm
- Peso: 180g (incluyendo batería)
- Correa: Nylon resistente UV, ajustable

### Autonomía
- Batería: LiPo 3.7V, 1200mAh o 1500mAh (capacidad POR DECIDIR)
- Autonomía teórica: unos 190 días con 1200mAh (calculado; unos 238 días con 1500mAh; sin medir)
- Tiempo de carga: por USB-C a 500mA, no menos de 2.4 horas con 1200mAh o 3 horas con 1500mAh; con panel solar POR DECIDIR (depende del panel, MPN pendiente; sin medir)

### Conectividad
- Wi-Fi: 2.4GHz para configuración inicial
- LoRa: 915MHz, alcance objetivo de más de 5km en campo abierto (sin medir)
- GNSS: solo L1; GPS, Galileo y BeiDou B1I por defecto, GLONASS configurable

### Resistencia Ambiental
- Grado de protección: IP67 (objetivo de diseño; pendiente de ensayo)
- Temperatura de operación: -20°C a +65°C (no se declara operación por encima de 65°C)
- Resistencia UV: Clasificación industrial

## Garantía y Soporte

### Garantía

El DR-VF-01 viene con una garantía de 2 años contra defectos de fabricación. Esta garantía cubre:
- Fallas en componentes electrónicos
- Problemas de fabricación
- Defectos en materiales

No cubre:
- Daños por uso indebido
- Modificaciones no autorizadas
- Desgaste normal de la correa

### Soporte Técnico
Para soporte técnico:
- Email: support@dr-vf-01.com
- Teléfono: +1-800-GEOFENCE (436-3362)
- Chat en vivo: www.dr-vf-01.com/support

Horario de atención: Lunes a Viernes, 8AM-6PM CST

### Actualizaciones
Visite regularmente www.dr-vf-01.com/updates para:
- Descargar nuevas versiones de firmware
- Acceder a mejoras de funcionalidad
- Obtener correcciones de seguridad

## Seguridad y Precaución

### Para Animales
- No utilice en animales menores a 6 meses
- Inspeccione regularmente la zona del cuello
- Retire inmediatamente si observa signos de incomodidad
- Consulte a un veterinario si hay irritación persistente

### Para Usuarios
- Evite exponer el dispositivo a agua salada por períodos prolongados
- No sumerja el collar en agua
- Mantenga alejado de fuentes de calor extremo
- No intente abrir la carcasa sellada

### Disposal
Este dispositivo contiene materiales electrónicos que deben desecharse adecuadamente:
- Contacte centros de reciclaje electrónicos locales
- No deseche en basura doméstica normal
- La batería debe ser removida por un centro autorizado antes de desechar el dispositivo

## Apéndice A: Cuidado del Panel Solar

### Limpieza
- Limpie la superficie solar con un paño suave y agua limpia
- Evite productos químicos abrasivos
- Limpie regularmente para mantener eficiencia óptima

### Posicionamiento
- Asegure que la superficie solar reciba luz directa
- Evite sombras permanentes de la correa u otros objetos
- Mantenga la superficie solar hacia arriba, en la parte superior del cuello, para máxima exposición diaria

## Apéndice B: Interpretación de Alertas

### Alertas Visuales
| Color LED | Estado | Acción Requerida |
|-----------|--------|------------------|
| Rojo fijo | Batería baja | Cargar inmediatamente |
| Rojo parpadeante | Alerta geocerca | Verificar posición animal |
| Amarillo parpadeante | Error del sistema | Contactar soporte |
| Verde/Azul alterno | Reinicio de fábrica | Esperar reinicio completo |

### Mensajes LoRa
Los mensajes enviados por LoRa incluyen:
- ID único del collar
- Coordenadas GPS actuales
- Estado de batería (porcentaje)
- Código de evento (normal/movimiento/alerta/error)

## Apéndice C: Consejos para Pastoreo Óptimo

### Configuración de Geocercas
- Incluya un margen de seguridad de 50 metros del perímetro físico
- Cree múltiples geocercas para áreas de pastoreo rotacional (función planificada; la versión actual admite un único polígono)
- Defina zonas de exclusión para áreas peligrosas (ríos, carreteras) (función planificada)

### Monitoreo de Animales
- Verifique alertas diariamente
- Preste atención a patrones de movimiento inusuales
- Documente comportamientos para mejorar manejo del hato

### Mantenimiento Preventivo
- Inspeccione físicamente cada collar mensualmente
- Reemplace correas cada 6 meses o según desgaste
- Actualice firmware trimestralmente

## Principios de Diseño Inspirados en Elecia White

### Facilidad de Uso
Este producto fue diseñado siguiendo los principios de "Making Embedded Systems" por Elecia White, priorizando:
- **Interfaz intuitiva**: Configuración simplificada con guía visual
- **Feedback claro**: Indicadores LED y mensajes de estado comprensibles
- **Documentación completa**: Manual detallado con ejemplos prácticos

### Robustez y Confiabilidad
- **Tolerancia a fallos**: El sistema continúa operando bajo condiciones anormales
- **Degradación controlada**: Funcionalidad limitada pero segura bajo fallos
- **Recuperación automática**: Reinicios y ajustes automáticos cuando es posible

### Mantenimiento Mínimo
- **Operación autónoma**: Diseñado para funcionar sin intervención frecuente
- **Diagnóstico automático**: Verificación continua del estado del sistema
- **Alertas proactivas**: Notificación anticipada de condiciones que requieren atención

Estos principios garantizan que el DR-VF-01 sea una solución confiable y fácil de usar para sus necesidades de pastoreo extensivo.

---

*Este manual está sujeto a cambios sin previo aviso. Consulte www.dr-vf-01.com/manual para la versión más reciente.*

© 2026 DR-VF-01 Technologies. Todos los derechos reservados.
