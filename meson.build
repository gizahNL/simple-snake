project('simplesnake', 'cpp')

sdl2 = dependency('sdl2')

sdl2font = dependency('SDL2_ttf')

executable('snake',
           'src/main.cpp',
           'src/game.cpp',
            dependencies: [sdl2, sdl2font]
)
