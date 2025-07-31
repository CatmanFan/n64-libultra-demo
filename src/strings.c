#include "strings.h"

char* strings[14];

/* ============= FUNCTIONS ============== */

static void add_string(int index, char* txt)
{
	strings[index] = txt;
}

void change_language()
{
	if (language < 0) language = 6;
	if (language > 6) language = 0;

	str_error = "THREAD %d (PR%d)\n%s";
	add_string(0, "Restarting...");
	add_string(1, "Loading...");
	add_string(2, "Language: English\nSelect a stage:");
	add_string(3, "Camera position (%c): %0.2f");
	add_string(4, "Camera FOV: %0.2f\n");
	add_string(5, "Object rotation (%c): %0.2f");
	add_string(6, "Object scale: %0.2f");
	add_string(7, "Frame %d\nTime: %0.2f\nSystem time: %0.2f\nFPS: %2d");
	add_string(8, "Controls:\nDPAD  = Rotate object\nCPAD  = Move camera dest.\nA/B   = Move camera Z-pos\nSTART = Fun surprise!\n");
	add_string(9, "DMA read test:");
	add_string(10, "Header read complete.");
	add_string(11, "ROM code: %s");
	add_string(12, "Header read failed");
	add_string(13, "Press B to return to menu");

	switch (language)
	{
		// ENGLISH
		default:
		case 0:
			str_error = "Fatal error, the software has been terminated.\nThread %d (priority: %d)";
			break;

		// FRENCH
		case 1:
			str_error = "Le logiciel a été arrêté des suites d'une erreur grave.\nProcessus léger : %d (priorité : %d)";
			add_string(0, "Redémarrage en cours ...");
			add_string(1, "Chargement en cours ...");
			add_string(2, "Langue : français\nChoisir un niveau :");
			add_string(3, "Position %c de la caméra : %0.2f");
			add_string(4, "Champ de vue de la caméra : %0.2f\n");
			add_string(5, "Rotation %c de l'objet : %0.2f");
			add_string(6, "Taille de l'objet : %0.2f");
			add_string(7, "Image n° %d\nTemps elapsé : %0.2f\nTemps du système : %0.2f\nFPS: %2d");
			add_string(8, "Commandes :\nDPAD  = Tourner l'objet\nCPAD  = Déplacer la destination de la caméra\nA/B   = Déplacer la caméra (Z)\nSTART = Une grande surprise !\n");
			add_string(9, "Essai de lecture avec DMA :");
			add_string(10, "Lecture de l'en-tête effectuée");
			add_string(11, "Code du jeu : %s");
			add_string(12, "Échec de lecture de l'en-tête");
			add_string(13, "Appuyez sur B pour retourner au menu");
			break;

		// SPANISH
		case 2:
			str_error = "Se ha cerrado el programa debido a un error grave.\nSubproceso n° %d (prioridad: %d)";
			add_string(0, "Se reiniciará el programa.");
			add_string(1, "Cargando...");
			add_string(2, "Idioma: español\nElige un nivel:");
			add_string(3, "Ubicación %c de la cámara: %0.2f");
			add_string(4, "Perspectiva de la cámara: %0.2f\n");
			add_string(5, "Rotación %c del objecto: %0.2f");
			add_string(6, "Tamaño del objecto: %0.2f");
			add_string(7, "Fotograma %d\nTiempo: %0.2f\nTiempo del sistema: %0.2f\nFPS: %2d");
			add_string(8, "Controles:\nDPAD  = Tornear el objecto\nCPAD  = Mover el destino de la cámara\nA/B   = Mover la cámara (Z)\nSTART = ¡Una gran sorpresa!\n");
			add_string(9, "Prueba de lectura con DMA:");
			add_string(10, "La lectura de la cabecera ha terminado con éxito.");
			add_string(11, "Código del juego: %s");
			add_string(12, "Error al leer la cabecera");
			add_string(13, "Pulsa B para volver al menú");
			break;

		// GERMAN
		case 3:
			str_error = "Die Software wurde beendet, weil ein schwerwiegender Fehler aufgetreten ist.\nThread %d / Priorität %d";
			add_string(0, "Die Software wird neu gestartet.");
			add_string(1, "Bitte warten...");
			add_string(2, "Spracheinstellung: Deutsch\nLevel auswählen:");
			add_string(3, "Kamerastellung (%c): %0.2f");
			add_string(4, "Kameraperspektive: %0.2f\n");
			add_string(5, "%c-Drehung des Objekts: %0.2f");
			add_string(6, "Skalieren des Objekts: %0.2f");
			add_string(7, "Bild %d\nZeit: %0.2f\nSystemzeit: %0.2f\nFPS: %2d");
			add_string(13, "Drücke B, um zum Menü zurückzukehren");
			break;

		// TURKISH
		case 4:
			str_error = "Bir kritik hata nedeniyle yazılım kapatıldı.\n%d. Iş Parçacığında (Önceliği: %d)";
			add_string(0, "Yazılım yeniden başlamaktadır.");
			add_string(1, "Lütfen bekleyin...");
			add_string(2, "Dil: Türkçe\nBir bölge seçin:");
			add_string(3, "Kameranın Mevkisi (%c): %0.2f");
			add_string(4, "Kameranın Görüş Alanı: %0.2f\n");
			add_string(5, "Obje Döndürme (%c): %0.2f");
			add_string(6, "Obje Boyutu: %0.2f");
			add_string(7, "Kare %d\nSüre: %0.2f\nSistem Süresi: %0.2f\nFPS: %2d");
			break;

		// TACHELHIT
		case 5:
			str_error = "Tazgelt icqqan, isala uṣnfaṛ.\nTizigzt tis-%d (tizwiri: %d)";
			add_string(0, "Rad isker daɣ uṣnfaṛ.");
			add_string(1, "Qql yan imikk ...");
			add_string(2, "Tutlayt: Taclḥiyt\nSti aswir:");
			add_string(3, "Tilawt n tkamirat (%c): %0.2f");
			add_string(4, "Tamdayt n tkamirat: %0.2f\n");
			add_string(5, "Amnid n tɣawsa (%c): %0.2f");
			add_string(6, "Tamɣuri n tɣawsa: %0.2f");
			add_string(7, "Tawlaft tis-%d\nTizi: %0.2f\nTizi n unagraw: %0.2f\nTiwlafin ku tazazit: %2d");
			add_string(8, "Commandes :\nDPAD  = Tourner l'objet\nCPAD  = Déplacer la destination de la caméra\nA/B   = Déplacer la caméra (Z)\nSTART = Une grande surprise !\n");
			add_string(9, "Essai de lecture avec DMA :");
			add_string(10, "Lecture de l'en-tête effectuée");
			add_string(11, "Code du jeu : %s");
			add_string(12, "Échec de lecture de l'en-tête");
			add_string(13, "Appuyez sur B pour retourner au menu");
			break;

		// JAPANESE
		case 6:
			str_error = "大変なエラーが発生したので、ソフトが終了しました。\\nスレッド%d(優先度%d)";
			add_string(0, "再起動しています。 ");
			add_string(1, "しばらくおまちください。 ");
			add_string(2, "言語:日本語\\nステージを選んでください:");
			add_string(3, "カメラの %cいち:%0.2f");
			add_string(5, "オブジェクトの %cかいてん:%0.2f");
			add_string(6, "オブジェクトの サイズ:%0.2f");
			add_string(7, "フレーム %d\nじかん:%0.2f\nシステムのじかん:%0.2f\nFPS:%2d");
			add_string(8, "コントロール:\nDPADで オブジェクト を かいてん\\nCPADで カメラの ほうこう を いどう\\nA,Bで カメラの Zいち を いどう ");
			add_string(9, "DMAでのよみこみテスト:");
			add_string(10, "ヘッダーをよみこみました ");
			add_string(11, "ROMコード:");
			add_string(12, "ヘッダーをよみこむことができません ");
			add_string(13, "メニューへもどるにはBボタンをおしてください ");
			break;
	}
};