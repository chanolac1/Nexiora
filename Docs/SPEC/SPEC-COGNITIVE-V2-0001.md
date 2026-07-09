# SPEC — COGNITIVE-V2-0001

## Comportamiento

- `ask <tema> "Que es X?"` detecta intención `definicion`.
- `ask <tema> "Para que sirve X?"` detecta intención `proposito`.
- `ask <tema> "Diferencia entre X e Y"` detecta intención `comparacion`.
- Si X existe en memoria, solo usa fragmentos que mencionen X.
- Si X no existe, intenta auto-investigación local.
