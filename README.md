# UpUP_Game
18년 1학기 시스템프로그래밍 UpUpGame


1. command line tool 설정

* `export PATH="/mergehex경로/:$PATH"`

* `export PATH="/nrfjprog경로/:$PATH"`


2. 설치 후 해당 
   ` example -> ble_periperal -> ble_app_blinky -> pca10040 -> s132 -> armgcc `
   
   `make` 로 **컴파일**
   
* ` nrfjprog --family nRF52 -e` (내용 clear)
 
*  `nrfjprog --family nRF52 --program s 어쩌고 softdevice.hex` (s132 softdevice 올리기)

*  `nrfjprog --family nRF52 --program _build/어쩌고.hex` (소스코드내용올리기)

*   `nrfjprog --family nRF52 -r` (실행시킴)
  
3. 건드린 부분 => 
* `makefile , main.c project_.c` project_로 시작하는 C 파일 생성


**nordic board, nrf52832 board**

게임 규칙 
1. 에너지게이트 다 없어지면 OUT
2. 벽돌 없는곳으로 점프(허공으로 점프) OUT
3. 맵은 점점 내려오는데 맵보다 아래있으면 OUT
