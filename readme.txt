
Для сборки требуется gcc-4.6 или выше, clang-3.0 или выше.

$ make

Для поддержания большого количества одновременных соединений - требуется запуск от рута.
Без этого максимальное количество определяется системными настройками - обычно 1000.

Имя файла на вход должно быть без пути, из него формируется url.

Поскольку http сервер крайне примитивен - он принимает только одну команду - GET.

$ ./httpserver readme.txt 
uri: / or /readme.txt
Content lenght: 918
Content-Type: text/plain; charset=utf-8
Server listen at port 7777, welcome.

Производительность:

$ ab -n 500000 -c 10 http://localhost:7777/readme.txt
Document Length:        1618 bytes
Concurrency Level:      10
Time taken for tests:   51.848 seconds
Complete requests:      500000
Requests per second:    9643.56 [#/sec] (mean)
Time per request:       0.104 [ms] (mean, across all concurrent requests)
Transfer rate:          16188.74 [Kbytes/sec] received

$ ab -n 500000 -c 1000 http://localhost:7777/readme.txt
Document Length:        1618 bytes
Concurrency Level:      1000
Time taken for tests:   59.011 seconds
Complete requests:      500000
Requests per second:    8473.05 [#/sec] (mean)
Time per request:       0.118 [ms] (mean, across all concurrent requests)
Transfer rate:          14223.81 [Kbytes/sec] received

