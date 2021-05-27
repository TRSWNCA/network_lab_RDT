# RDT实验
```
RDT/
├── CMakeLists.txt                  #CMakeLists
├── README.md                       #info
├── cmake-build-debug               #CMake Build
├── doc                             #实验报告
├── log                             #运行结果
│   ├── goBackN.log                 #Go-Back-N运行结果
│   └── stopAndWait.log             #Stop-and-Wait运行结果
├── manual                          #实验手册
└── src                             #源文件
    ├── RDT_goBackN_ver.c           #RDT: Go-Back-N实现
    ├── RDT_stopAndWait_ver.c       #RDT: Stop-and-Wait实现
    ├── prog2.c                     #初始提供的框架源文件
    ├── rdt_1.c                     #rdt1.0
    ├── rdt_2_0.c                   #rdt2.0
    ├── rdt_2_1.c                   #rdt2.1
    ├── rdt_2_2.c                   #rdt2.2
    └── rdt_3.c                     #rdt3.0
```
- Stop-and-Wait实现(modern): `src/RDT_stopAndWait_modern_ver.c`

- Stop-and-Wait实现(K&R C): `src/RDT_stopAndWait_KRC_ver.c`

- Go-Back-N实现(modern): `src/RDT_goBackN_modern_ver.c`

- Go-Back-N实现(K&R C): `src/RDT_goBackN_KRC_ver.c`