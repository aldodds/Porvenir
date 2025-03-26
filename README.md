# Propósito del repositorio

Este repositorio contiene los códigos, las notas, imágenes y documentos de los dispostivos del sistema de monitoreo ambiental y edáfico

## Estructura del Proyecto  

El proyecto está organizado en las siguientes carpetas:  

- **`/Codigo`** → Contiene el código fuente para Arduino y otros scripts.  
- **`/Texto`** → Contiene documentación y anotaciones 
- **`/Mixto`** → Carpeta que contiene archivos que combinan código y documentación.  

## APN Claro Colombia

La información de la [APN de la SIM Card de Claro colombia](https://selectra.com.co/empresas/claro/celulares/apn) se establece en el código de la siguiente forma:

```C++
  // Configurar APN de Claro Colombia
  modem.gprsConnect("internet.comcel.com.co", "comcel", "comcel");
```