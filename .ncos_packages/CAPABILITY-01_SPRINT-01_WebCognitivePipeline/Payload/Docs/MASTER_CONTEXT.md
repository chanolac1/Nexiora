# MASTER CONTEXT
# Proyecto Nexiora

Versión del contexto: 1.0

Última actualización:
Después de EPIC-03 Sprint-01

---

# Visión

Nexiora es un Sistema Operativo Cognitivo Autónomo desarrollado completamente en C23.

No es un chatbot.

No es un LLM.

No es un framework.

Su objetivo es convertirse en una inteligencia artificial capaz de:

- aprender
- investigar
- razonar
- planificar
- crear software
- autocorregirse
- evolucionar continuamente

Todo ello bajo un modelo determinista, explicable y seguro.

---

# Filosofía

Toda decisión del proyecto debe cumplir:

✔ Modularidad

✔ Seguridad

✔ Explicabilidad

✔ Trazabilidad

✔ Determinismo cuando sea necesario

✔ Evolución continua

✔ Sin deuda técnica

✔ Cero warnings

✔ Cien por ciento de pruebas

✔ Todo automatizado

Nunca se aceptan soluciones temporales.

Nunca se aceptan reparaciones superficiales.

Toda falla encontrada genera:

- corrección de raíz
- prueba de regresión
- documentación

---

# Lenguaje

Lenguaje principal:

C23

Compilador:

MSYS2 UCRT64 GCC

Build:

CMake

Ninja

---

# Arquitectura

Nexiora está dividido en grandes motores.

CORE

Motor base.

COGNITIVE

Investigación
Memoria
Aprendizaje

NCOS

Sistema Operativo Cognitivo.

TOOLS

Herramientas CLI.

KNOWLEDGE

Conocimiento persistente.

PACKAGE MANAGER

Instalación
Rollback
Versionado

---

# Estado actual

Implementado

CORE

Cognitive Core

Learning

Knowledge Base

Concept Registry

Concept Graph

Reasoner

Answer Composer

Intent Planner

Session Engine

Planning Engine

Task Engine

Compiler Engine

Build Log Analyzer

Auto Fix Proposal

Safe Patch

Patch Approval

Patch Apply

Patch Verification

Package Manager

Dependency Resolver

Transactional Install

Rollback

Package History

Workspace Engine

Multimodal Ingestion Foundation

---

# Package Manager

El Package Manager es el único mecanismo oficial para instalar módulos.

No se permiten cambios manuales.

Todo Sprint debe entregarse como paquete instalable.

Flujo:

verify

↓

deps

↓

install

↓

build

↓

tests

↓

history

↓

rollback

Todos los paquetes deben ser:

idempotentes

versionados

reversibles

documentados

---

# Reglas de calidad

Todo código nuevo debe cumplir:

0 errores

0 warnings

100 % tests

sin memory leaks

sin UB

sin hacks

sin fixes manuales

Cada bug produce:

una prueba nueva

una corrección permanente

---

# Roadmap

EPIC 01

Autodesarrollo

Sprint 01

Patch Generator

...

Sprint 18

Package History

Sprint 19

Workspace Engine

Multimodal Ingestion Foundation

Pendiente

Sprint 20

Self Validation Engine

Sprint 21

Patch Ranking

Sprint 22

Evolution Score

Sprint 23

Candidate Promotion

Sprint 24

Autonomous Development Loop

---

EPIC 02

Memoria de Largo Plazo

Objetivo:

Memoria episódica

Memoria semántica

Memoria procedimental

Consolidación

Olvido inteligente

Versionado

---

EPIC 03

Multimodalidad

Objetivo:

PDF

Word

Excel

PowerPoint

Imágenes

OCR

Audio

Video

Repositorios Git

Bases de datos

Web

---

EPIC 04

Agentes Desarrolladores

Objetivo:

Crear proyectos completos

Crear videojuegos

Crear APIs

Crear librerías

Compilar

Corregir

Optimizar

Documentar

Publicar

---

EPIC 05

Evolución Autónoma

Objetivo:

Detectar problemas

Investigar

Aprender

Generar soluciones

Validar

Medir

Clasificar

Solicitar aprobación

Integrar

Aprender del resultado

---

# Objetivo final

Construir un sistema capaz de evolucionar continuamente y desarrollar software de forma autónoma, o responder dudas de cualquier indole, con razonamiento para las soluciones, segura y explicable.

Cada nueva capacidad debe aumentar la inteligencia del sistema y permitirle construir capacidades aún más complejas.

Nexiora deberá llegar a ser capaz de crear sistemas completos, incluyendo videojuegos, aplicaciones empresariales y nuevos módulos para sí mismo.
## Capacidad multimodal incorporada después de EPIC-03 Sprint-02

Nexiora dispone de una capa de adaptadores locales verificables para OCR de imágenes, extracción de audio desde video y transcripción mediante modelos Whisper locales. Ninguna herramienta externa se considera disponible sin descubrimiento y ninguna ejecución se considera exitosa sin evidencia textual generada.

## Capacidad incorporada después de EPIC-03 Sprint-03

Nexiora puede transformar conocimiento multimodal normalizado en evidencia fragmentada, conceptos, relaciones y respuestas fundamentadas. La comprensión sigue siendo simbólica y determinista; la visión directa y el razonamiento temporal avanzado continúan pendientes.

## Enfoque por capacidades

Nexiora evoluciona mediante capacidades observables. CAPABILITY-01 unifica entrada, percepción, conocimiento, razonamiento y aprendizaje. Las fuentes web deben conservar procedencia y pasar por compuertas de evidencia antes de incorporarse al sistema cognitivo.
