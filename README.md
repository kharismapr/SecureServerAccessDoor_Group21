# IOT 21 Secure Server Access Door

```
Kharisma Aprilia - 2306223244
Stevie Nathania Siregar - 2306242382
Mirza Adi Raffiansyah - 2306210323
Ruben Kristanto - 2306214624
```

## Introduction
Sistem keamanan pada ruang data center membutuhkan pengelolaan akses yang terkontrol serta pemantauan kondisi lingkungan secara berkelanjutan. Dengan memanfaatkan teknologi IoT, fungsi-fungsi tersebut dapat digabungkan dalam satu perangkat yang bekerja otomatis dan dapat dipantau dari jarak jauh. Proyek ini mengembangkan Smart Door Lock berbasis ESP32 yang mengintegrasikan autentikasi RFID, akses darurat melalui Bluetooth Low Energy (BLE), pemantauan suhu ruangan, dan pencatatan aktivitas ke platform cloud. Sistem dijalankan menggunakan FreeRTOS agar setiap proses dapat beroperasi secara paralel dengan stabil, serta dilengkapi mekanisme notifikasi untuk memberikan peringatan ketika terjadi anomali baik pada akses maupun kondisi lingkungan.

## Implementatoin
Implementasi sistem dilakukan dengan mengintegrasikan ESP32 sebagai pusat kendali dengan modul RFID RC522, sensor DHT11, LCD I2C, serta koneksi Wi-Fi dan BLE. Setiap komponen disusun mengikuti skema komunikasi yang sesuai, seperti SPI untuk RFID dan I2C untuk LCD, sementara sensor suhu dan LED indikator terhubung melalui pin GPIO. Pada sisi perangkat lunak, sistem dijalankan menggunakan FreeRTOS agar pembacaan kartu, pemantauan suhu, dan koneksi ke Blynk dapat berjalan bersamaan tanpa saling mengganggu, dengan Mutex digunakan untuk mengatur penggunaan LCD. Seluruh proses kemudian dihubungkan dengan Google Sheets melalui HTTP Request untuk mencatat log akses, sehingga keseluruhan fungsi dapat bekerja secara terkoordinasi dalam satu sistem yang stabil.

## Testing ang Evaluation
Pengujian dilakukan untuk memastikan seluruh fungsi pada sistem Secure Server Access Door berjalan seperti yang dirancang, mulai dari validasi akses RFID, pemantauan suhu, hingga pengiriman notifikasi dan pencatatan log. Pengujian kartu unauthorized menunjukkan sistem mampu menolak akses dengan benar, menampilkan pesan “ACCESS DENIED” pada LCD, dan mengirimkan notifikasi ke Blynk secara real-time. Sebaliknya, kartu yang terdaftar berhasil dikenali dan memicu tampilan “ACCESS ALLOWED” serta menyalakan LED sebagai simulasi pintu terbuka. Pengujian sensor DHT11 juga menunjukkan bahwa suhu dapat ditampilkan secara berkala pada LCD, dan sistem merespons kondisi overheat dengan memicu event peringatan di aplikasi Blynk. Selain itu, verifikasi data pada Google Sheets memastikan bahwa setiap percobaan akses terekam dengan akurat melalui HTTP Request, sehingga keseluruhan pengujian menunjukkan bahwa integrasi perangkat keras dan perangkat lunak berfungsi dengan stabil dan sesuai spesifikasi.

![image](https://hackmd.io/_uploads/ByRT4t4Gbl.png)

![image](https://hackmd.io/_uploads/Hks0NK4Mbe.png)

Implementasi sistem Secure Server Access Door dilakukan melalui integrasi antara ESP32 sebagai pengendali utama dengan berbagai komponen seperti RFID RC522, sensor suhu DHT11, LCD I2C, LED indikator, serta modul Wi-Fi dan BLE. Setiap komponen dihubungkan menggunakan protokol komunikasi yang sesuai, seperti SPI untuk RFID dan I2C untuk LCD, sementara pembacaan suhu dan kontrol LED memanfaatkan GPIO. Di sisi perangkat lunak, FreeRTOS digunakan untuk menjalankan beberapa task secara paralel, termasuk pembacaan kartu, pemantauan suhu, dan koneksi jaringan. Mekanisme Mutex diterapkan untuk mengatur penggunaan LCD agar tidak terjadi konflik antar-task, sedangkan modul HTTPClient digunakan untuk mengirim log akses ke Google Sheets. Implementasi ini memungkinkan seluruh fungsi baik kontrol akses, monitoring, maupun notifikasi berjalan simultan dan terkoordinasi dengan baik pada satu sistem terintegrasi.

![image](https://hackmd.io/_uploads/rJVGBYNMWg.png)

![image](https://hackmd.io/_uploads/H1l7SYEM-x.png)

## Conclusion
Proyek Smart Server Access Door ini adalah sebuah demonstrasi yang sukses mengenai bagaimana teknologi IoT berbiaya rendah dapat diterapkan untuk mengatasi kebutuhan keamanan infrastruktur kritis. Kami berhasil menyatukan komponen hardware (sensor, indikator, antarmuka) dengan kendali mikrokontroler ESP32 yang berkemampuan tinggi, menghasilkan sebuah sistem yang kohesif. Pelajaran utama yang didapat dari proyek ini adalah pemahaman praktis tentang kompromi antara kinerja, keandalan nirkabel, dan efisiensi daya. Meskipun sistem ini telah mencapai tujuan awalnya, potensi pengembangannya sangat luas. Ke depannya, sistem ini dapat diperluas dengan menambahkan log data historis yang lebih rinci, atau mengintegrasikan sensor kualitas udara yang lebih canggih, menjadikannya bukan hanya alat keamanan, tetapi juga alat manajemen fasilitas yang komprehensif. Proyek ini membuktikan bahwa konsep keamanan IoT yang berlapis adalah masa depan pengamanan ruang server.

## Referensi
[1] Espressif Systems, “ESP32 Series Datasheet,” Version 5.2, Espressif Systems, Shanghai, China, 2025. [Online]. Available: https://documentation.espressif.com/esp32_datasheet_en.pdf. [Accessed: Dec. 8, 2025].
[2] Blynk Team, “Blynk Documentation: Getting Started with ESP32,” Blynk IoT Platform Documentation, [Online]. Available: https://docs.blynk.io/en/getting-started/what-do-i-need. [Accessed: Dec. 8, 2025].
[3] R. N. Tutorials, “Security Access using MFRC522 RFID Reader with Arduino,” Random Nerd Tutorials, [Online]. Available: https://randomnerdtutorials.com/security-access-using-mfrc522-rfid-reader-with-arduino/. [Accessed: Dec. 8, 2025].
[4] C. Basics, “How to Set Up the DHT11 Humidity Sensor on an Arduino,” Circuit Basics, [Online]. Available: https://www.circuitbasics.com/how-to-set-up-the-dht11-humidity-sensor-on-an-arduino/. [Accessed: Dec. 8, 2025].
