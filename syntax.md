# Resumo da Sintaxe Bulang

A Bulang é uma linguagem de script dinâmica focada no desenvolvimento de jogos 2D, com suporte nativo para corrotinas (processos), classes, módulos e uma API de engine integrada (rendering, física, som, input).

---

## 1. Variáveis e Tipos

Tipagem dinâmica. Usa-se `var` para declarar variáveis. Tipos suportados: números (int/float), strings, booleanos, `nil`, listas (arrays) e instâncias de classes/processos.

```bulang
var vida = 100;
var nome = "Player";
var velocidade = 12.5;
var ativo = true;
var vazio = nil;
var lista = [];
var cor = Color(255, 0, 0, 255);  // Struct nativo
var pos = Vec2(10, 20);           // Struct nativo
```

### Operadores
- Aritméticos: `+`, `-`, `*`, `/`, `%`
- Comparação: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Lógicos: `&&`, `||`, `!`
- Atribuição composta: `+=`, `-=`, `*=`, `/=`
- Concatenação de strings: `+`

---

## 2. Funções (`def`)

Funções são definidas com `def`. Devem ser definidas **antes** de serem chamadas (exceto dentro de classes).

```bulang
def calcular_dano(base, bonus)
{
    var total = base + bonus;
    return total;
}

var d = calcular_dano(10, 5);
```

### Retorno Múltiplo
Funções podem retornar múltiplos valores, capturados com desestruturação:

```bulang
def find_free_spot(_x, _y)
{
    return (_x, _y, 1);  // retorna 3 valores
}

var (fx, fy, ok) = find_free_spot(100, 200);
```

---

## 3. Processos (`process`)

Processos são corrotinas especiais que representam entidades no jogo. Possuem propriedades nativas e mantêm o estado entre frames.

### Propriedades Nativas (sem `var`)
| Propriedade | Descrição |
|---|---|
| `x`, `y` | Posição no mundo (pixels) |
| `z` | Ordem de desenho (layer/profundidade) |
| `angle` | Rotação em graus |
| `size` | Escala do sprite (padrão: 1.0) |
| `life` | Variável auxiliar (HP, tempo de vida) |
| `state` | Variável inteira auxiliar (máquinas de estado) |
| `graph` | ID do sprite/gráfico atual |
| `id` | Identificador único do processo (leitura) |
| `flip_x`, `flip_y` | Espelhamento do sprite (booleano) |
| `visible` | Se o processo é desenhado (booleano) |

### Palavras-chave Especiais
- **`frame;`** — Pausa a execução até ao próximo frame do jogo.
- **`loop { ... }`** — Ciclo infinito (ideal para comportamento por frame).
- **`break;`** — Sai do loop e termina o processo.
- **`let_me_alone();`** — Mata todos os outros processos.

### Propriedades Personalizadas (Públicas)
Atribuições **sem `var`** dentro do processo tornam-se propriedades de instância acessíveis externamente:

```bulang
process puck(start_x, start_y)
{
    x = start_x; y = start_y;
    velx = 0.0; vely = 0.0;   // propriedades públicas personalizadas
    radius = 14;               // acessível via p.radius

    set_circle_shape(radius);

    loop
    {
        var dt = delta();       // 'var' = local/privada
        x += velx * dt;
        y += vely * dt;

        // Render
        set_color(20, 20, 20);
        draw_circle(x, y, radius, true);

        frame;
    }
}

// Instanciar e aceder
var p = puck(100, 200);
// Aceder propriedades: p.x, p.velx, p.radius
```

### Referência a Processos
```bulang
var p_id = puck(100, 200);     // retorna o ID
var p = proc(p_id);            // obtém handle/instância
if (p) { p.velx = 50; }       // acede propriedades
```

---

## 4. Classes (`class`)

A Bulang suporta classes completas com construtor, métodos, herança e `self`.

### Classe Básica
```bulang
class NoteDef
{
    var solfege;
    var letter;
    var number;

    def init(_solfege, _letter, _number)
    {
        self.solfege = _solfege;
        self.letter = _letter;
        self.number = _number;
    }
}

var nota = NoteDef("Do", "C", "1");
print(nota.letter);  // "C"
```

### Herança (`:`)
```bulang
class Screen
{
    var name;
    var active;

    def init(_name)
    {
        self.name = _name;
        self.active = false;
    }

    def enter() { }
    def leave() { }
    def update(_dt) { }
}

class MenuScreen : Screen
{
    var menu_index;

    def init()
    {
        super.init("Menu");   // chama construtor da classe pai
        self.menu_index = 0;
    }

    def enter()
    {
        print("Entered menu");
    }
}
```

### Métodos e `self`
- Dentro de métodos, usar `self.propriedade` para aceder membros.
- `super.metodo()` para chamar método da classe pai.
- Valores por defeito em `var` são permitidos: `var playing = false;`

### Instanciação
Classes são instanciadas chamando o nome diretamente (sem `new`):
```bulang
var tween = Tween(0, 100, 1.0, "sine_out");
tween.update(dt);
```

---

## 5. Structs (`struct`)

Structs são tipos de dados leves, **sem métodos**, apenas com campos nomeados. São ideais para agrupar dados simples sem a complexidade de uma `class`.

### Definição
Os campos podem ser declarados com ou sem `var`, e separados por vírgula ou em linhas separadas:

```bulang
struct Ponto
{
    x, y
}

// ou com var (mais explícito)
struct Rect
{
    var x;
    var y;
    var w;
    var h;
}

// múltiplos campos por linha com vírgula
struct Config
{
    var width, height;
    var title;
}
```

### Instanciação
Cria-se uma instância chamando o nome da struct com os valores posicionais dos campos (da mesma ordem em que foram declarados). Campos não fornecidos ficam `nil`:

```bulang
var p = Ponto(10, 20);       // x=10, y=20
var r = Rect(0, 0, 800, 600); // x=0, y=0, w=800, h=600
var c = Config(1280, 720);   // width=1280, height=720, title=nil
```

### Acesso e Modificação de Campos
```bulang
print(p.x);       // 10
print(p.y);       // 20

p.x = 50;         // modifica o campo
p.y = p.y + 10;
```

### Structs vs Classes
| | `struct` | `class` |
|---|---|---|
| Campos | Sim | Sim |
| Métodos | **Não** | Sim |
| Construtor `init` | Não (posicional) | Sim (`def init(...)`) |
| Herança | **Não** | Sim (`:`) |
| `self` | **Não** | Sim |
| Uso ideal | Dados simples | Entidades com lógica |

### Exemplo Prático
```bulang
struct Enemy
{
    var type;
    var hp;
    var speed;
}

var enemies = [];
enemies.push(Enemy("goblin", 30, 1.5));
enemies.push(Enemy("dragon", 200, 0.8));

var i = 0;
while (i < len(enemies))
{
    var e = enemies[i];
    print(format("{}: HP={} Speed={}", e.type, e.hp, e.speed));
    i += 1;
}
```

---

## 5b. Sintaxe Genérica (`<Type>`)

Bulang suporta sintaxe de tipo genérico ao estilo C#. Quando se usa `<Type>` numa chamada de função ou método, o nome do tipo é passado como **primeiro argumento** (string).

### Chamada de função com tipo genérico

```bu
def createComponent(typeName, value)
{
    print(typeName);   // "Health"
    return value;
}

var hp = createComponent<Health>(100);
// Equivalente a: createComponent("Health", 100)
```

### Método de classe com tipo genérico

```bu
class Entity
{
    var components;

    def init()
    {
        self.components = {};
    }

    def addComponent(typeName, value)
    {
        self.components[typeName] = value;
    }

    def getComponent(typeName)
    {
        return self.components[typeName];
    }
}

var e = Entity();
e.addComponent<Health>(100);     // → e.addComponent("Health", 100)
var hp = e.getComponent<Health>(); // → e.getComponent("Health")
```

### Construtor de classe com tipo genérico

```bu
class Container
{
    var typeName;
    var data;

    def init(typeName, data)
    {
        self.typeName = typeName;
        self.data = data;
    }
}

var c = Container<Items>(42);
// Equivalente a: Container("Items", 42)
```

### Regras

| Padrão | Desugaring |
|--------|-----------|
| `foo<T>(a, b)` | `foo("T", a, b)` |
| `obj.method<T>(a)` | `obj.method("T", a)` |
| `Class<T>(a)` | `Class("T", a)` |
| `foo<T>()` | `foo("T")` |

> **Nota:** O tipo entre `<>` é passado como string. Os operadores `<` e `>` continuam a funcionar normalmente em comparações. A sintaxe genérica é reconhecida apenas quando o padrão `<Identificador>(` aparece imediatamente após um identificador ou nome de método.

---

## 6. Estruturas de Controlo

### If / Elif / Else
```bulang
if (vida > 50)
{
    print("Saudável");
}
elif (vida > 0)
{
    print("Ferido");
}
else
{
    print("Morto");
}
```

### While
```bulang
var i = 0;
while (i < 10)
{
    print(i);
    i += 1;
}
```

### For
```bulang
for (var i = 0; i < 10; i += 1)
{
    print(i);
}
```

### Switch / Case
Não é necessário `break` — sem fall-through automático.

```bulang
switch (tipo)
{
    case 0:
        print("Tipo Zero");
    case 1:
        print("Tipo Um");
    default:
        print("Outro Tipo");
}
```

---

## 6. Listas (Arrays)

Listas dinâmicas indexadas a zero. Podem conter tipos mistos.

```bulang
var inimigos = [];

// Adicionar
inimigos.push(10);
inimigos.push("texto");
inimigos.push([1, 2, 3]);   // listas aninhadas

// Aceder e modificar
var e = inimigos[0];
inimigos[0] = 99;

// Tamanho
var n = len(inimigos);

// Limpar
inimigos.clear();

// Iterar
var i = 0;
while (i < len(inimigos))
{
    print(inimigos[i]);
    i += 1;
}

// Listas como "structs" ligeiros
var ponto = [10, 20];       // x, y
var x = ponto[0];
ponto[1] = 30;
```

---

## 7. Módulos e Includes

### `import` — Módulos com Prefixo
```bulang
import math;
import os;
import fs;
import json;

var r = math.irand(0, 10);
var plat = os.platform;
var raw = fs.read("data.json");
var obj = json.parse(raw);
```

### `using` — Acesso Direto (sem prefixo)
```bulang
import math;
using math;
var r = irand(0, 10);   // sem prefixo math.
```

### `include` — Incluir Outros Scripts
```bulang
include "stages.bu";
include "tween.bu";
```

### `require` — Plugins Nativos (C/C++)
```bulang
require "sdl,net";
```

---

## 8. Strings e Formatação

```bulang
var nome = "Player";
var msg = "Olá " + nome;     // concatenação com +
var n = len(msg);              // comprimento

// format() com placeholders {}
var texto = format("Score: {}", SCORE);
var info = format("Deliveries: {}/{}", entregas, total);
var debug = format("Pos: ({}, {})", int(x), int(y));
```

---

## 9. Conversões e Utilitários

```bulang
var i = int(3.7);       // 3 (trunca para inteiro)
var s = str(42);         // "42"
var n = len(lista);      // tamanho de lista ou string
```

---

## 10. Funções Matemáticas Globais

```bulang
var a = abs(-5);                // 5
var s = sin(1.57);              // ~1.0
var c = cos(0);                 // 1.0
var r = sqrt(16);               // 4.0

// Via módulo math
var rand = math.irand(0, 100);  // inteiro aleatório
var rf = math.rand(0.0, 1.0);   // float aleatório
```

---

## 11. Engine — Funções Essenciais

### Tempo
```bulang
var dt = delta();    // tempo desde o último frame (segundos)
var t = time();      // tempo total de execução
```

### Input (Teclado)
```bulang
if (key_down(KEY_UP))     { }   // tecla mantida
if (key_pressed(KEY_SPACE)) { } // tecla acabou de ser pressionada
if (key_released(KEY_A))  { }   // tecla acabou de ser largada
```

### Input (Rato)
```bulang
var mx = get_mouse_x();
var my = get_mouse_y();
if (mouse_down(0)) { }           // botão esquerdo mantido
if (mouse_pressed(0)) { }        // botão acabou de ser pressionado
var (dx, dy) = get_mouse_delta();
```

### Input (Touch/Mobile)
```bulang
var count = touch_count();
var tx = get_touch_screen_x(0);
var ty = get_touch_screen_y(0);
var tid = get_touch_id(0);
```

### Virtual Keys (Touch D-Pad)
```bulang
vkey_clear();
vkey_add(KEY_LEFT, x, y, w, h);    // mapeia região do ecrã a uma tecla
vkey_add(KEY_SPACE, bx, by, bw, bh);
vkey_set_visible(true);
```

### Janela e Resolução
```bulang
set_window_size(1280, 720);
set_window_title("Meu Jogo");
set_design_resolution(1280, 720);
set_virtual_screen_enabled(true);
set_screen_scale_mode(3);           // 0=fit, 1=stretch, 2=fill, 3=letterbox
```

### Câmara
```bulang
set_scroll(cam_x, cam_y);
set_camera_zoom(1.5);
set_camera_target(player.x, player.y);
start_camera_shake(4, 4, 10, 8);
```

### Desenho (Draw)
```bulang
set_draw_screen(true);              // screen-space (HUD)
set_color(255, 200, 100);
set_alpha(200);
draw_rectangle(10, 10, 200, 50, true);  // true=preenchido
draw_circle(100, 100, 30, false);        // false=contorno
draw_line(0, 0, 100, 100);
draw_text("Hello", 10, 10, 24);
draw_triangle(0, 0, 50, 0, 25, 40, true);
set_draw_screen(false);             // volta a world-space
```

### Gráficos/Sprites
```bulang
var g = load_graph("assets/player.png");
var atlas = load_atlas("assets/tiles.png", 8, 8);
draw_graph(g, 100, 200);
draw_graph_ex(g, 100, 200, 45, 2.0, 2.0, false, false);
```

### Som e Música
```bulang
var snd = load_sound("assets/hit.wav");
play_sound(snd, 1.0, 1.0);          // volume, pitch

var mus = load_music("assets/bg.ogg");
play_music(mus);
set_music_volume(mus, 0.5);
update_music_streams();              // chamar 1x por frame!
```

### Colisão (Sistema Simples)
```bulang
// Dentro de um process:
set_rect_shape(0, 0, 40, 20);
set_circle_shape(30);
set_collision_layer(1);
set_collision_mask(1);

if (place_free(x + 1, y)) { x += 1; }
var hit = collision("enemy", x, y);
move_and_slide(vx, vy);
```

### Processos — Gestão
```bulang
var p = proc(PLAYER_ID);      // obtém handle por ID
if (exists(PLAYER_ID)) { }    // verifica se existe
signal(enemy_id, SKILL);      // envia sinal
var e = get_id("enemy");      // primeiro processo do tipo
let_me_alone();                // mata todos os outros processos
```

### Física Box2D
```bulang
create_world(0, 9.8);
var bdef = create_bodydef(BODY_DYNAMIC);
bdef.set_position(100, 200);
var body = create_body(bdef);
body.add_box(30, 20);
body.apply_impulse(100, -200);
update_physics(dt);
```

### Partículas (Emitters)
```bulang
var e = create_emitter(false, 0, 50);
e.set_position(100, 200);
e.set_speed_range(20, 80);
e.set_life(0.5);
e.set_color_curve(Color(255, 200, 100, 255), Color(255, 50, 0, 0));
e.set_spread(6.28);

// Presets prontos:
create_explosion(x, y, 0, Color(255, 100, 0, 255));
create_smoke(x, y, 0);
create_fire(x, y, 0);
```

### Ficheiros e JSON
```bulang
import fs;
import json;

var raw = fs.read("data.json");
var obj = json.parse(raw);
```

### Imagens (Manipulação)
```bulang
var img = Image(128, 128);
img.fill(0, 0, 0, 255);
img.set_pixel(10, 10, Color(255, 0, 0, 255));
var graph_id = img.to_graph("meu_sprite");
```

---

## 12. Comentários

```bulang
// Comentário de linha

/* Comentário
   de bloco */
```

---

## 13. Estrutura Típica de um Jogo

```bulang
import math;
import os;

// Constantes e variáveis globais
var VIEW_W = 1280;
var VIEW_H = 720;
var PLAYER_ID = -1;

// Funções utilitárias (definidas antes do uso)
def clamp(v, min_v, max_v)
{
    if (v < min_v) return min_v;
    if (v > max_v) return max_v;
    return v;
}

// Processos (entidades do jogo)
process player(start_x, start_y)
{
    x = start_x;
    y = start_y;
    velx = 0.0;
    vely = 0.0;

    set_rect_shape(0, 0, 32, 32);

    loop
    {
        var dt = delta();

        if (key_down(65)) velx = -200;   // A
        if (key_down(68)) velx = 200;    // D
        x += velx * dt;
        velx *= 0.9;

        // Render
        set_color(100, 200, 255);
        draw_circle(x, y, 16, true);

        frame;
    }
}

// Bootstrap — código de topo executado ao iniciar
set_window_size(VIEW_W, VIEW_H);
set_window_title("Meu Jogo");
set_design_resolution(VIEW_W, VIEW_H);
set_virtual_screen_enabled(true);

PLAYER_ID = player(VIEW_W / 2, VIEW_H / 2);

// Main loop (quando não se usa processos como loop principal)
loop
{
    var dt = delta();
    set_draw_screen(true);
    set_color(20, 24, 36);
    draw_rectangle(0, 0, VIEW_W, VIEW_H, true);
    // ... lógica e desenho ...
    set_draw_screen(false);
    frame;
}
```

---

## 14. Resumo Rápido de Palavras-Chave

| Palavra-chave | Uso |
|---|---|
| `var` | Declarar variável (local/privada) |
| `def` | Definir função |
| `process` | Definir processo (corrotina/entidade) |
| `class` | Definir classe (com métodos/herança) |
| `struct` | Definir struct (dados sem métodos) |
| `self` | Referência à instância atual (em classes) |
| `super` | Referência à classe pai |
| `if` / `elif` / `else` | Condicionais |
| `while` | Ciclo condicional |
| `for` | Ciclo com inicialização/condição/incremento |
| `switch` / `case` / `default` | Seleção por valor |
| `loop` | Ciclo infinito |
| `frame` | Yield até ao próximo frame |
| `break` | Sair do ciclo |
| `return` | Retornar valor(es) de função |
| `import` | Importar módulo (com prefixo) |
| `using` | Usar módulo sem prefixo |
| `include` | Incluir ficheiro `.bu` |
| `require` | Carregar plugin nativo |
| `true` / `false` | Booleanos |
| `nil` | Valor nulo |