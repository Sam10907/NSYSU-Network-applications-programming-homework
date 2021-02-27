# FTP program with data compression
實作FTP client端與server端，使client端可以傳送檔案到server端，檔案是經過huffman壓縮演算法壓縮後再傳送，並在server端解壓縮

huffman.h是自定義的函式庫，支援壓縮與解壓縮的功能，使用可變長度的編碼方式進行壓縮

huffman.cpp是實作檔案

最後在client.cpp和server.cpp中引入此自定義函式庫，進行檔案的加壓縮與解壓縮
