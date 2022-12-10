# FEUP SCI

## General Info

This project was created during the **1st semester** of the **2nd year** of the **Master in Informatics and Computing Engineering**, in **Cyberphysical Systems and Internet of Things** curricular unity (FEUP).

[SCI FEUP-Sigarra](https://sigarra.up.pt/feup/en/UCURR_GERAL.FICHA_UC_VIEW?pv_ocorrencia_id=501969 "Curricular Unity Homepage")

## About the Project

This project's name is **Inventory Management** and represents a prototype of a possible CS-IoT platform that integrates ESP32 devices ([M5 Atom Lite](https://docs.m5stack.com/en/core/atom_lite)), AWS IoT Core services and an Outsystems application.

Please see more information on the folder [doc](doc/).

## Setup

### 1. AWS IoT Core services

Create an account on AWS and a project on AWS IoT Core services. Configure it properly.

### 2. Arduino

Open the files [device1.ino](device1/device1.ino), [device2.ino](device2/device2.ino) and [device3.ino](device3/device3.ino) and change the placeholders *TODO: Change* to the values of your choice.

Upload each one of these programs into your ESP32 (M5 Atom Lite) devices. Please take into account that the [device1.ino](device1/device1.ino) program uses the TVOC/eCO2 Gas Sensor Unit (SGP30), while the other two use the ENV III Unit with Temperature, Humidity, and Air Pressure Sensor (SHT30+QMP6988). Change also the ID variable in each program, if needed.

By doing this, your devices will be ready.

### 3. Outsystems

For this part, you'll need to have an Outsystems account and Service Studio installed on your PC.

Go to the Forge (Outsystems store) and install in your environment the [MQTT Mobile Client module](https://www.outsystems.com/forge/component-overview/1944/mqtt-mobile-client).

Upload the Outsystems Application Package available at [Inventory Management.oap](app/Inventory%20Management.oap) into your Outsystems environment. 

Open the application, go to the module *InventoryManagement1*, then go to the *Interface* tab and then to the *AWSBlock*, and change the variables in the assignment of the *OnInitialize* action to the ones of your AWS IoT Core project. 

Publish the module on your environment and test the application on your browser or generate the Android or iOS application to test it on your mobile phone.

---

**Note:** For further guidance, please consult the following web pages:

- [AWS IoT Core setup](https://how2electronics.com/connecting-esp32-to-amazon-aws-iot-core-using-mqtt/)
- [AWS Get Access and Secret Keys](https://medium.com/serverlessguru/guide-first-serverless-project-630b91366505)



