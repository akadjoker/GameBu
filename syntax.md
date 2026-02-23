# Resumo da Sintaxe Bulang

A Bulang é uma linguagem de script dinâmica focada no desenvolvimento de jogos, com suporte nativo para corrotinas (processos) e gestão de entidades.

## 1. Variáveis e Tipos
A tipagem é dinâmica. Usa-se `var` para declarar variáveis.

```bulang
var vida = 100;
var nome = "Player";
var velocidade = 12.5;
var ativo = true;
var lista = [];
```

## 2. Funções (`def`)
As funções são definidas com `def`.
**Nota Importante:** As funções devem ser definidas **antes** de serem chamadas no código (ordem de definição importa).

```bulang
def calcular_dano(base, bonus)
{
    var total = base + bonus;
    return total;
}

// Chamada
var d = calcular_dano(10, 5);
```

## 3. Processos (`process`)
Processos são corrotinas especiais que representam entidades no jogo. Eles possuem propriedades nativas e mantêm o seu estado entre frames.

### Propriedades Nativas
*   **`x`, `y`**: Posição no mundo (pixels).
*   **`z`**: Ordem de desenho (Layer/Profundidade).
*   **`angle`**: Rotação em graus.
*   **`size`**: Escala do sprite (padrão: 1.0).
*   **`life`**: Variável auxiliar (frequentemente usada para tempo de vida ou HP).
*   **`state`**: Variável inteira auxiliar (frequentemente usada para máquinas de estados).
*   **`graph`**: ID do sprite/gráfico atual.
*   **`id`**: Identificador único do processo (Leitura).
*   **`flip_x`, `flip_y`**: Espelhamento do sprite (booleano).
*   **`visible`**: Define se o processo é desenhado (booleano).

*   **`frame;`**: Pausa a execução do processo até ao próximo frame do jogo.
*   **`loop`**: Cria um ciclo infinito, ideal para o comportamento da entidade.

```bulang
process inimigo(start_x, start_y)
{
    // Propriedades nativas (não precisam de 'var')
    x = start_x;
    y = start_y;
    z = 10;
    
    // Setup inicial
    set_rect_shape(0, 0, 32, 32);

    loop
    {
        // Lógica por frame
        y += 100 * delta(); // Move para baixo
        
        if (y > 600) break; // Sai do loop e mata o processo
        
        frame; // Entrega o controlo à engine e volta aqui no próximo frame
    }
}
```

## 4. Estruturas de Controlo

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

### Switch / Case
A estrutura `switch` seleciona um bloco baseado no valor.
**Nota:** Não é necessário usar `break`. O fluxo não passa para o próximo caso (no fall-through) automaticamente.

```bulang
var tipo = 1;

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

## 5. Listas (Arrays)
Listas dinâmicas indexadas a zero.

```bulang
var inimigos = [];

// Adicionar
inimigos.push(10);
inimigos.push(20);

// Aceder
var e = inimigos[0];

// Limpar (Recomendado em vez de criar nova lista)
inimigos.clear();

// Iterar
var i = 0;
while (i < len(inimigos))
{
    print(inimigos[i]);
    i += 1;
}
```

