with open("bild.bin", "rb") as file:
    # Lese Breite und Höhe des Bildes aus den ersten beiden Bytes
    breite = int.from_bytes(file.read(1), byteorder='big')
    hoehe = int.from_bytes(file.read(1), byteorder='big')

    # Lese Pixel-Daten aus der restlichen Datei
    pixel_bytes = file.read()
    pixel_data = []
    for byte in pixel_bytes:
        for i in range(8):
            pixel_data.append(byte >> i & 1)

    # Gebe das Bild als ASCII-Art aus
    for y in range(breite):
        for x in range(hoehe):
            index = x * breite + y
            pixel = pixel_data[index]
            if pixel == 1:
                print("#", end="")
            else:
                print("_", end="")
        print() # Neue Zeile