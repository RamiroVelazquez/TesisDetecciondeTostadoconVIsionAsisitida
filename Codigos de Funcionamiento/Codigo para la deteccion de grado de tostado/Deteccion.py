# Realizamos las importaciones de todas las librerías
import cv2
import os
import numpy as np
from ultralytics import YOLO
from skimage.feature import graycomatrix, graycoprops

# Cargamos el modelo para la detección de los granos de cacao, previamente entrenado para esto
modelo = YOLO('C:/Users/vepro/OneDrive/Escritorio/DeteccionDeCacao/DetectorDeCacao/cacao_detector.pt')

# Leemos una imagen
imagen = cv2.imread('C:/Users/vepro/OneDrive/Escritorio/DeteccionDeCacao/DetectorDeCacao/images66.jpg')
imagen = cv2.resize(imagen, (600, 600))

# Realizar la detección con el modelo
cacaos = modelo(imagen)
# Crear un directorio para guardar las imágenes de los granos de cacao
directorioCacaosDetectados = 'cacao_images'
os.makedirs(directorioCacaosDetectados, exist_ok=True)

# Lista para almacenar los valores numéricos de la clasificación de tostado
ValorDeTostado = []

# Función para clasificar el grado de tostado basado en el promedio de RGB
def ClasificarPorColor(RGBRed, RGBGreen, RGBBlue):
    # Clasificación basada en color
    if RGBRed > 160 and RGBGreen > 120 and RGBBlue > 60:  # Café claro
        return 1  # Poco tostado
    elif RGBRed > 120 and RGBGreen > 80 and RGBBlue > 40:  # Café medio
        return 2  # Medio tostado
    else:  # Café oscuro
        return 3  # Muy tostado

# Función para clasificar el grado de tostado basado en la textura (contrast y homogeneity)
def ClasificadorPorTextura(contraste, homogeneidad):
    # Ajuste basado en textura
    if contraste > 8 and homogeneidad < 0.4:
        return 3  # Muy tostado
    elif contraste > 5 and homogeneidad < 0.6:
        return 2  # Medio tostado
    else:
        return 1  # Poco tostado

# Iterar sobre los cacaos detectados
for i, box in enumerate(cacaos[0].boxes.xyxy):
    x1, y1, x2, y2 = map(int, box)  # Coordenadas del bounding box

    # Extraer la región de interés (ROI) correspondiente a cada bounding box
    CacaoIndividual = imagen[y1:y2, x1:x2]

    # Convertir la sub-imagen a escala de grises
    CacaoEnGris = cv2.cvtColor(CacaoIndividual, cv2.COLOR_BGR2GRAY)

    # Aplicar umbralización para obtener una máscara binaria
    _, umbral = cv2.threshold(CacaoEnGris, 120, 255, cv2.THRESH_BINARY_INV)

    # Encontrar los contornos dentro de la ROI (segmentación)
    contornos, _ = cv2.findContours(umbral, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # Dibujar los contornos sobre la imagen original
    cv2.drawContours(CacaoIndividual, contornos, -1, (0, 255, 0), 2)
    
    # Calcular los promedios de los canales R, G y B
    RGBRed = np.mean(CacaoIndividual[:, :, 2])  # Canal R (rojo)
    RGBGreen = np.mean(CacaoIndividual[:, :, 1])  # Canal G (verde)
    RGBBlue = np.mean(CacaoIndividual[:, :, 0])  # Canal B (azul)

    # Convertir la sub-imagen a escala de grises para el análisis de textura
    CacaoEnGris = cv2.cvtColor(CacaoIndividual, cv2.COLOR_BGR2GRAY)

    # Calcular la matriz de co-ocurrencia (GLCM) para el análisis de textura
    glcm = graycomatrix(CacaoEnGris, distances=[1], angles=[0], levels=256, symmetric=True, normed=True)

    # Extraer propiedades de la textura
    contraste = graycoprops(glcm, 'contrast')[0, 0]
    homogeneidad = graycoprops(glcm, 'homogeneity')[0, 0]

    # Clasificación basada en color y textura
    ColorDeTostado = ClasificarPorColor(RGBRed, RGBGreen, RGBBlue)
    TexturaDeTostado = ClasificadorPorTextura(contraste, homogeneidad)

    # Combinación ponderada (70% color, 30% textura)
    TostadoFinal = (0.7 * ColorDeTostado) + (0.3 * TexturaDeTostado)

    # Clasificación final
    if TostadoFinal < 1.5:
        GradoDeTostado = 'Cacao poco tostado'
        NumeroDeTostado = 1
    elif 1.5 <= TostadoFinal < 2.5:
        GradoDeTostado = 'Cacao medio tostado'
        NumeroDeTostado = 2
    else:
        GradoDeTostado = 'Cacao muy tostado'
        NumeroDeTostado = 3

    # Guardar el valor de la clasificación
    ValorDeTostado.append(NumeroDeTostado)

    # Guardar la sub-imagen segmentada y clasificada
    GuardadoDeCacao = os.path.join(directorioCacaosDetectados, f'cacao_{i}_{GradoDeTostado}.png')
    cv2.imwrite(GuardadoDeCacao, CacaoIndividual)
    
    #print(f"Grano {i}: {GradoDeTostado}, Promedio RGB: R={RGBRed}, G={RGBGreen}, B={RGBBlue}, Contrast={contraste}, Homogeneity={homogeneidad}")
    print(f"Grano {i}: {GradoDeTostado}")

# Calcular el promedio de la calidad de tostado
if ValorDeTostado:
    PromedioDeTostado = sum(ValorDeTostado) / len(ValorDeTostado)
    print(f"\nPromedio de la calidad de tostado: {PromedioDeTostado}")
    if PromedioDeTostado < 1.5:
        print("Resultado final: La mayoría de los granos están poco tostados.")
    elif 1.5 <= PromedioDeTostado < 2.5:
        print("Resultado final: La mayoría de los granos están medianamente tostados.")
    else:
        print("Resultado final: La mayoría de los granos están muy tostados.")
else:
    print("No se detectaron granos de cacao.")
