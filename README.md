## Dependências Externas

Este projeto utiliza o DirectXTK como submódulo git. Para clonar e inicializar corretamente:

```sh
git clone --recursive <repo-url>
```

Se já clonou o repositório, inicialize os submódulos com:

```sh
git submodule update --init --recursive
```

O código-fonte do DirectXTK estará em `extern/DirectXTK`. 