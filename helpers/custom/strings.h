#ifndef __language_C__
#define __language_C__

	#define EN  0 // English
	#define FR  1 // French
	#define ES  2 // Spanish
	#define DE  3 // German
	#define TR  4 // Turkish
	#define RU  5 // Russian
	#define JA  6 // Japanese
	#define SHI 7 // Tachelhit

	extern int language;
	extern char *STR_STAGE;
	extern char *STR_FRAME;
	extern char *STR_ERROR;
	extern char *STR_REBOOT;

	extern void change_language();

	/*#if   language == FR
	// ****************************
	// Strings: French
	// ****************************
	#define STR_STAGE	"Langue : français\nChoisir un niveau : %d"
	#define STR_FRAME	"Image n° %d"
	#define STR_CAMPOS	"Position %s de la caméra : %1f"
	#define STR_OBJROT	"Angle %s de l'objet :      %1f"
	#define STR_REBOOT	"Redémarrage en cours ..."
	#define STR_ERROR	"Erreur grave au thread n° %d (priorité fixée à %d).\nLe logiciel a été arrêté.\n\n" \
						"Code de l'erreur : %s\n\n" \
						"Paramètres :\n" \
						"pc       0x%8x\n" \
						"badvaddr 0x%8x\n"

	#elif language == ES
	// ****************************
	// Strings: Spanish
	// ****************************
	#define STR_STAGE	"Idioma: español\nElige un nivel: %d"
	#define STR_FRAME	"Fotograma %d"
	#define STR_CAMPOS	"Ubicación %s de la cámara: %1f"
	#define STR_OBJROT	"Rotación %s del objecto:  %1f"
	#define STR_REBOOT	"Reiniciando..."
	#define STR_ERROR	"Error grave en el subproceso n° %d (prioridad fijada a %d).\nSe ha cerrado el programa.\n\n"\
						"Código del error: %s\n\n"\
						"Parametros:\n"\
						"pc       0x%8x\n"\
						"badvaddr 0x%8x\n"

	#elif language == DE
	// ****************************
	// Strings: German
	// ****************************
	#define STR_STAGE	"Spracheinstellung: Deutsch\nLevel auswählen: %d"
	#define STR_FRAME	"Bild %d"
	#define STR_CAMPOS	"Kamerastellung (%s):     %1f"
	#define STR_OBJROT	"Objektsperspektive (%s): %1f"
	#define STR_REBOOT	"Die Software wird neu gestartet."
	#define STR_ERROR	"Schwerwiegender Fehler im Thread %d mit der Priorität %d.\nDie Software wurde beendet.\n\n"\
						"Nachricht: %s\n\n"\
						"Einzelnachweise:\n"\
						"pc       0x%8x\n"\
						"badvaddr 0x%8x\n"

	#elif language == TR
	// ****************************
	// Strings: Turkish
	// ****************************
	#define STR_STAGE	"Dil: Türkçe\nBir bölge seçin: %d"
	#define STR_FRAME	"Kare %d"
	#define STR_CAMPOS	"Kameranın %s-Mevkisi: %1f"
	#define STR_OBJROT	"Objenin %s-Açısı:     %1f"
	#define STR_REBOOT	"Yazılım yeniden başlamıyor."
	#define STR_ERROR	"%d. iş parçacığında (önceliği: %d) bir kritik hata oluştu.\nYazılım kapatıldı.\n\n"\
						"Hata kodu: %s\n\n"\
						"Parametreler:\n"\
						"pc       0x%8x\n"\
						"badvaddr 0x%8x\n"

	#elif language == RU
	// ****************************
	// Strings: Russian
	// ****************************
	#define STR_STAGE	"Язык: русский\nВыберите уровень: %d"
	#define STR_FRAME	"Kaдp %d"
	#define STR_CAMPOS	"Camera %s-pos: %1f"
	#define STR_CAMROT	"Camera %s-rot: %1f"
	#define STR_REBOOT	"Пepeзaгpузкa"
	#define STR_ERROR	"Кpитичecкaя oшибкa\nв тpeдe %d (пpиopитeт: %d)\n"\
						"%s\n\n"\
						"Пpoгpaммa зaкpытa."*/

#endif