import serial
import sys
import time
import numpy

# constante multiplicativa para obter a quantidade em segundos a partir de ml
const_ml_to_ms=0.4155

def calculate_colors(quantity,cyan_code,magenta_code,yellow_code,black_code):
    base_code = 0
    base = 0
    cyan = 0
    magenta = 0
    yellow = 0
    black = 0

    # verifica se as cores sao iguais
    if cyan_code == magenta_code and magenta_code == yellow_code :
        # se o codigo das 3 cores for igual, a cor desejada e preto
        base=ml_to_ms(0)
        cyan=ml_to_ms(0)
        magenta=ml_to_ms(0)
        yellow=ml_to_ms(0)
        black=ml_to_ms(quantity)
    else:
        # porcentagem de cores (excluindo a base)
        percent_colors=cmyk_total_percent(cyan_code,magenta_code,yellow_code,black_code)

        # calcula complemento de base
        base_code = base_complement(percent_colors)

        # calcula percentagem total
        percent_total = percent_colors+base

        # calcula mls (divide preto por 4)
        base=ml_to_ms(cmyk_to_ml(base_code,percent_total))
        cyan=ml_to_ms(cmyk_to_ml(cyan_code,percent_total))
        magenta=ml_to_ms(cmyk_to_ml(magenta_code,percent_total))
        yellow=ml_to_ms(cmyk_to_ml(yellow_code,percent_total))
        black=ml_to_ms(cmyk_to_ml(black_code,percent_total))

    # envia pro arduino
    send_color(base,cyan,magenta,yellow,black)

def base_complement(percent_colors):
    if percent_colors >= 100:
        base = 0
    else:
        base = numpy.invert(int(percent_colors)-100)
    return base

# calcula a percentagem total de tinta
def cmyk_total_percent(cyan_code,magenta_code,yellow_code,black_code):
    percent_total=cyan_code+magenta_code+yellow_code+black_code
    return float(percent_total)

# quantidade em ml e igual a porcentagem parcial (de cada tinta) * 100
# dividido pela porcentagem total (soma de todas as tintas)
def cmyk_to_ml(percent_of_color,percent_total):
    color_in_ml=(100*float(percent_of_color))/float(percent_total)
    return int(color_in_ml)

# milliliter_to_milliseconds
def ml_to_ms(qt_ml):
    time=qt_ml*const_ml_to_ms*1000
    # tempo em milisegundos para ligar a bomba
    return int(time)

def send_color(base,cyan,magenta,yellow,black):
    ser = serial.Serial(port='/dev/ttyUSB0', baudrate=9600)
    time.sleep(1)
    # base = 5000
    data = "{},{},{},{},{}\n".format(str(base),str(cyan),str(magenta),str(yellow),str(black))
    print(data)
    ser.write(data.encode())
    time.sleep(2)
    output = ''
    while not "finish" in output:
        output = str(ser.readline())
        print('output:' + output)
    ser.close()

def main(quantity,cyan_code,magenta_code,yellow_code,black_code):
    calculate_colors(quantity,cyan_code,magenta_code,yellow_code,black_code)

if __name__ == '__main__':
    main(int(sys.argv[1]),int(sys.argv[2]),int(sys.argv[3]),int(sys.argv[4]),int(sys.argv[5]))