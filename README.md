# Lines-of-Action
Notes on the creation of Lines of Action

## Docker container to compile C to WebAssembly
Create the container.
```
sudo docker build -t emscripten-c .
```

Confirm its existence.
```
sudo docker images
```

Kill the container.
```
sudo docker image rm emscripten-c
```

## Zobrist hash generator

## Client-facing game logic module

## Citation
If this code was helpful to you, please cite this repository.

```
@misc{linesofaction,
  title={Lines of Action in C},
  author={Eric C. Joyce},
  year={2025},
  publisher={Github},
  journal={GitHub repository},
  howpublished={\url{https://github.com/EricCJoyce/Lines-of-Action}}
}
```
