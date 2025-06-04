import serial
import time
import threading

arduino_port = 'COM8'
esp32_port = 'COM3'
baud_rate = 115200
baud_rate2 = 9600

arduinoenviar = serial.Serial(arduino_port, baud_rate, timeout=1)
arduinorecibir = arduinoenviar
esp32 = serial.Serial(esp32_port, baud_rate, timeout=1)

def enviar_comandos():
    while True:
        comando_usuario = input("Escribe un comando para enviar: ").strip()
        if comando_usuario:
            arduinoenviar.write((comando_usuario + '\n').encode())
            print(f"Comando enviado manualmente: {comando_usuario}")

# Ejecutar `enviar_comandos()` en un hilo separado
threading.Thread(target=enviar_comandos, daemon=True).start()

print("Esperando comandos del Arduino...")


while True:
    try:
        comando = arduinorecibir.readline().decode().strip()
        print(f"EStado Nestlé: {comando}")
        # Verificar si el mensaje empieza y termina con '*'
        if comando.startswith("*") and comando.endswith("*"):
            # Extraer el contenido sin los asteriscos y reemplazar las comas por espacios
            comando_limpio = comando.strip('*').replace(',', ' ')
            print(f"Comando transformado y enviado a ESP32: {comando_limpio}")
            
#rs100/0/0/0/0/0/64/
            esp32.write((comando_limpio + '\n').encode())
            
            respuesta_esp32 = esp32.readline().decode().strip()
            if respuesta_esp32:
                print(f"Respuesta recibida de ESP32: {respuesta_esp32}")
    except Exception as e:
        print(f"Error: {e}")