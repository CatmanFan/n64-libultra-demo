/* ============= PROTOTYPES ============= */

int language;

char *str_error;
char *str_00;
char *str_01;
char *str_02;
char *str_03;
char *str_04;
char *str_05;
char *str_06;

/* ============= FUNCTIONS ============== */

void change_language()
{
	if (language < 0) language = 6;
	if (language > 6) language = 0;

	str_error = "";
	str_00 = "";
	str_01 = "";
	str_02 = "";
	str_03 = "";
	str_04 = "";
	str_05 = "";
	str_06 = "";

	switch (language)
	{
		// ENGLISH
		case 0:
			str_error = "Fatal error at thread %d (priority %d).\n" \
						"Software terminated.\n\n" \
						"Error code\n%s\n\n" \
						"Parameters (Page %d: L/R)";
			str_00 = "Restarting...";

			str_01 = "Language: English\nSelect a stage: %d";

			str_02 = "Frame %d\nTime: %0.2f\nFPS: %2d";

			str_03 = "Camera position (%c): %0.2f";
			str_04 = "Camera FOV: %0.2f\n";
			str_05 = "Object rotation (%c): %0.2f";
			str_06 = "Object scale: %0.2f";
			break;

		// FRENCH
		case 1:
			str_error = "Erreur grave au thread n° %d (priorité fixée à %d).\n" \
						"Le logiciel a été arrêté.\n\n" \
						"Code de l'erreur\n%s\n\n" \
						"Paramètres (Page %d : L / R)";
			str_00 = "Redémarrage en cours ...";

			str_01 = "Langue : français\nChoisir un niveau : %d";

			str_02 = "Image n° %d\nTemps elapsé : %0.2f\nFPS: %2d";

			str_03 = "Position %c de la caméra : %0.2f";
			str_04 = "Champ de vue de la caméra : %0.2f\n";
			str_05 = "Rotation %c de l'objet : %0.2f";
			str_06 = "Taille de l'objet : %0.2f";
			break;

		// SPANISH
		case 2:
			str_error = "Error grave en el subproceso n° %d (prioridad fijada a %d).\n" \
						"Se ha cerrado el programa.\n\n" \
						"Código del error\n%s\n\n" \
						"Parametros (%d °a página: L/R)";
			str_00 = "Se reiniciará el programa.";

			str_01 = "Idioma: español\nElige un nivel: %d";

			str_02 = "Fotograma %d\nTiempo: %0.2f\nFPS: %2d";

			str_03 = "Ubicación %c de la cámara: %0.2f";
			str_04 = "Perspectiva de la cámara: %0.2f\n";
			str_05 = "Rotación %c del objecto: %0.2f";
			str_06 = "Tamaño del objecto: %0.2f";
			break;

		// GERMAN
		case 3:
			str_error = "Schwerwiegender Fehler im Thread %d mit der Priorität %d.\n" \
						"Die Software wurde beendet.\n\n" \
						"Fehlercode\n%s\n\n" \
						"Parameter (Seite %d: L/R)";
			str_00 = "Die Software wird neu gestartet.";

			str_01 = "Spracheinstellung: Deutsch\nLevel auswählen: %d";

			str_02 = "Bild %d\nZeit: %0.2f\nFPS: %2d";

			str_03 = "Kamerastellung (%c): %0.2f";
			str_04 = "Kameraperspektive: %0.2f\n";
			str_05 = "%c-Drehung des Objekts: %0.2f";
			str_06 = "Skalieren des Objekts: %0.2f";
			break;

		// TURKISH
		case 4:
			str_error = "%d. iş parçacığında (önceliği: %d) bir kritik hata oluştu.\n" \
						"Yazılım kapatıldı.\n\n" \
						"Hata Kodu\n%s\n\n" \
						"Parametreler (%d. Sayfa: L/R)";
			str_00 = "Yazılım yeniden başlamıyor...";

			str_01 = "Dil: Türkçe\nBir bölge seçin: %d";

			str_02 = "Kare %d\nSüre: %0.2f\nFPS: %2d";

			str_03 = "Kameranın Mevkisi (%c): %0.2f";
			str_04 = "Kameranın Görüş Alanı: %0.2f\n";
			str_05 = "Obje Döndürme (%c): %0.2f";
			str_06 = "Obje Boyutu: %0.2f";
			break;

		// TACHELHIT
		case 5:
			str_error = "Tazgelt icqqan ɣ tɣrist %d (tizwiri: %d).\n" \
						"Isala uṣnfaṛ.\n\n" \
						"Uṭṭun n tzgelt\n%s\n\n" \
						"Tifawin (tasna tis-%d: L/R)";
			str_00 = "Rad isker daɣ uṣnfaṛ.";

			str_01 = "Tutlayt: Taclḥiyt\nSti aswir: %d";

			str_02 = "Tawlaft %d\nTizi: %0.2f\nTiwlafin ku tazazit: %2d";

			str_03 = "Tilawt n tkamirat (%c): %0.2f";
			str_04 = "Tamdayt n tkamirat: %0.2f\n";
			str_05 = "Amnid n tɣawsa (%c): %0.2f";
			str_06 = "Tamɣuri n tɣawsa: %0.2f";
			break;

		// JAPANESE
		case 6:
			str_error = "スレッド%d(優先度%d)には大変なエラーが発生。\" /* " */ \
						"\nソフトが終了しました。\\n\n" \
						"エラーコード\\n%s\n\n" \
						"くわしく (ページ%d:L/R)";
			str_00 = "再起動しています。\\n";

			str_01 = "言語:日本語\\nステージを選んでください:%d";

			str_02 = "フレーム %d\nじかん:%0.2f\nFPS:%0.2f";

			str_03 = "カメラのばしょ(%c):%0.2f";
			str_05 = "オブジェクトのしてん(%c):%0.2f";
			str_06 = "オブジェクトのサイズ:%0.2f";
			break;
	}
};