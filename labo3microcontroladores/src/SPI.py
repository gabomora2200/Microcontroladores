import serial

baud = 9600 #sample rate
filename = "datos.csv" #Nombre del archivo de texto que se va a crear 
samples = 29 # va a tomar 5 mediciones de tension 

file = open(filename, 'w') #Crea el archivo de texto
file.close()# cierra el archivo de texto
print("Created file")


ser = serial.Serial("/tmp/ttyS1", baud) #Abre la comunicacion con el arduino usando el puerto ttyS1
print("Conectado")
file = open(filename, 'w') # Crea un archivo con el nombre dato
print("Created file")

line = 0
contador  = 0
datos = []


while (1): # hace un loop infinito en el cual se esperan datos en todo momento 
    getData = str(ser.readline()) #Lee el puerto 
    data = getData[2:][:-5] # Se quita la basura que vienen en los datos
    print(data)

    if contador == 4: #Logica de acomodo de los datos para generar el CSV. 
        file = open(filename, "a")
        file.write(data + "\n")
        contador = 0
    else:
        file.write(data + ",")
        contador+=1
