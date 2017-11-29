#ifndef BrainBWin_H
#define BrainBWin_H

/**
 * @brief Benchmarking Cognitive Abilities of the Brain with Computer Games
 *
 * @file BrainBWin.h
 * @author  Norbert Bátfai <nbatfai@gmail.com>
 * @version 0.0.1
 *
 * @section LICENSE
 *
 * Copyright (C) 2017 Norbert Bátfai, nbatfai@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 */

#include <QKeyEvent>
#include <QMainWindow>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QFile>
#include <QString>
#include <QCloseEvent>
#include <QDate>
#include <QDir>
#include <QDateTime>

/*#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>*/
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <algorithm>

#include "BrainBThread.h"

#include "PlayerDataHolder.h"

enum playerstate {
    lost,
    found
};

class BrainBWin : public QMainWindow
{
    Q_OBJECT

    BrainBThread *brainBThread;
    QPixmap pixmap;
    Heroes *heroes;

    int mouse_x;
    int mouse_y;
    int yshift {50};
    int nofLost {0};
    int nofFound {0};

    bool firstLost {false};
    bool start {false};
    playerstate state = lost;
    std::vector<int> lost2found;
    std::vector<int> found2lost;

    QString statDir;

public:
    static const QString appName;
    static const QString appVersion;
    BrainBWin ( int w = 256, int h = 256, QWidget *parent = 0 );

    void closeEvent ( QCloseEvent *e ) {

        if ( save ( brainBThread->getT() ) ) {
            brainBThread->finish();
            e->accept();
        } else {
            e->ignore();
        }


    }

    virtual ~BrainBWin();
    void paintEvent ( QPaintEvent * );
    void keyPressEvent ( QKeyEvent *event );
    void mouseMoveEvent ( QMouseEvent *event );
    void mousePressEvent ( QMouseEvent *event );
    void mouseReleaseEvent ( QMouseEvent *event );

    double mean ( std::vector<int> vect ) {

        double sum = std::accumulate ( vect.begin (), vect.end (), 0.0 );
        return  sum / vect.size();

    }

    double var ( std::vector<int> vect, double mean ) {

        double accum = 0.0;

        std::for_each ( vect.begin (), vect.end (), [&] ( const double d ) {
            accum += ( d - mean ) * ( d - mean );
        } );

        return sqrt ( accum / ( vect.size()-1 ) );
    }


    void millis2minsec ( int millis, int &min, int &sec ) {

        sec = ( millis * 100 ) / 1000;
        min = sec / 60;
        sec = sec - min * 60;

    }

	/*void serviceRequestFinished(QNetworkReply*) {

	}*/

	void sendDataToServer() {
		CURL *curl;
		CURLcode res;

		/* In windows, this will init the winsock stuff */
		curl_global_init(CURL_GLOBAL_ALL);

		/* get a curl handle */
		curl = curl_easy_init();
		if (curl) {
			/* First set the URL that is about to receive our POST. This URL can
			just as well be a https:// URL if that is what should receive the
			data. */
			curl_easy_setopt(curl, CURLOPT_URL, "http://postit.example.com/moo.cgi");
			/* Now specify the POST data */
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");

			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);
			/* Check for errors */
			if (res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));

			/* always cleanup */
			curl_easy_cleanup(curl);
		}
		curl_global_cleanup();
	}

	size_t static write_callback_func(char *contents, size_t size, size_t nmemb, void *userp){
		((std::string*)userp)->append((char*)contents, size * nmemb);
		return size * nmemb;
	}

	double eliminateNan(double number) {
		return number == number ? number : 0;
	}

    bool save ( int t ) {

        bool ret = false;

        if ( !QDir ( statDir ).exists() )
            if ( !QDir().mkdir ( statDir ) ) {
                return false;
            }

        QString name = statDir + "/Test-" + QString::number ( t );
        QFile file ( name + "-screenimage.png" );
        if ( file.open ( QIODevice::WriteOnly ) ) {
            ret = pixmap.save ( &file, "PNG" );
        }

        QFile tfile ( name + "-stats.txt" );
        ret = tfile.open ( QIODevice::WriteOnly | QIODevice::Text );
        if ( ret ) {
            QTextStream textStremam ( &tfile );

			int lostMean, foundMean, lost2foundMean, found2lostMean;
			double lostVar, foundVar, lost2foundVar, found2lostVar;

            textStremam << appName + " " + appVersion << "\n";
            textStremam << "time      : " <<  brainBThread->getT() << "\n";
            textStremam << "bps       : " <<  brainBThread->get_bps() << "\n";
            textStremam << "noc       : " <<  brainBThread->nofHeroes() << "\n";
            textStremam << "nop       : " <<  brainBThread->get_nofPaused() << "\n";

            textStremam << "lost      : "  << "\n";
            std::vector<int> l = brainBThread->lostV();
            for ( int n : l ) {
                textStremam << n << ' ';
            }
            textStremam <<  "\n";
            int m = mean ( l );
            textStremam << "mean      : " <<  m << "\n";
            textStremam << "var       : " <<  var ( l, m ) << "\n";

			lostMean = m;
			lostVar = var(l, m);

            textStremam << "found     : " ;
            std::vector<int> f = brainBThread->foundV();
            for ( int n : f ) {
                textStremam << n << ' ';
            }
            textStremam <<  "\n";
            m = mean ( f );
            textStremam << "mean      : " <<  m << "\n";
            textStremam << "var       : " <<  var ( f, m ) << "\n";

			foundMean = m;
			foundVar = var(f, m);

            textStremam << "lost2found: "  ;
            for ( int n : lost2found ) {
                textStremam << n << ' ';
            }
            textStremam <<  "\n";
            int m1 = m = mean ( lost2found );
            textStremam << "mean      : " <<  m << "\n";
            textStremam << "var       : " <<  var ( lost2found, m ) << "\n";

			lost2foundMean = m;
			lost2foundVar = var(lost2found, m);

            textStremam << "found2lost: "  ;
            for ( int n : found2lost ) {
                textStremam << n << ' ';
            }
            textStremam <<  "\n";
            int m2 = m = mean ( found2lost );
            textStremam << "mean      : " <<  m << "\n";
            textStremam << "var       : " <<  var ( found2lost, m ) << "\n";

			found2lostMean = m;
			found2lostVar = var(found2lost, m);

            if ( m1 < m2 ) {
                textStremam << "mean(lost2found) < mean(found2lost)" << "\n";
            }

            int min, sec;
            millis2minsec ( t, min, sec );
            textStremam << "time      : " <<  min  << ":"  << sec << "\n";

	    double res = ( ( ( (double)m1+(double)m2 ) /2.0 ) /8.0 ) /1024.0;
            textStremam << "U R about " << res << " Kilobytes\n";

            tfile.close();

			lostVar = eliminateNan(lostVar);
			foundVar = eliminateNan(foundVar);
			lost2foundVar = eliminateNan(lost2foundVar);
			found2lostVar = eliminateNan(found2lostVar);
			std::string lostVarString = std::to_string(lostVar);
			std::replace(lostVarString.begin(), lostVarString.end(), ',', '.');
			std::string foundVarString = std::to_string(foundVar);
			std::replace(foundVarString.begin(), foundVarString.end(), ',', '.');
			std::string lost2foundMeanString = std::to_string(lost2foundVar);
			std::replace(lost2foundMeanString.begin(), lost2foundMeanString.end(), ',', '.');
			std::string found2lostMeanString = std::to_string(found2lostVar);
			std::replace(found2lostMeanString.begin(), found2lostMeanString.end(), ',', '.');
			std::string kilobytes = std::to_string(res);
			std::replace(kilobytes.begin(), kilobytes.end(), ',', '.');

			std::string data = "{\"player\": {\"name\": \"" + PlayerDataHolder::Name + "\", \"gender\": \"" + PlayerDataHolder::Gender
				+ "\", \"age\": \"" + std::to_string(PlayerDataHolder::Age) + "\", \"plays_regularly\": \"" + std::to_string(PlayerDataHolder::PlaysRegularly)
				+ "\", \"is_esport_athlete\": \"" + std::to_string(PlayerDataHolder::IsEsportAthlete) + "\"}, \"measurement_data\": {\"bps\": "
				+ std::to_string(brainBThread->get_bps()) + ", \"noc\": " + std::to_string(brainBThread->nofHeroes()) + ", \"nop\": " + std::to_string(brainBThread->get_nofPaused())
				+ ", \"lost_mean\": " + std::to_string(lostMean) + ", \"lost_var\": " + lostVarString
				+ ", \"found_mean\": " + std::to_string(foundMean) + ", \"found_var\": " + foundVarString
				+ ", \"lost2found_mean\": " + std::to_string(lost2foundMean) + ", \"lost2found_var\": " + lost2foundMeanString
				+ ", \"found2lost_mean\": " + std::to_string(found2lostMean) + ", \"found2lost_var\": " + found2lostMeanString
				+ ", \"kilobytes\": " + kilobytes
				+ "}}";
				
				/*"name=\"NULL\"&gender=\"NULL\"&age=\"NULL\"&plays_regularly=\"NULL\"&is_esport_athlete=\"NULL\"&bps="
				+ std::to_string(brainBThread->get_bps()) + "&nop=" + std::to_string(brainBThread->get_nofPaused())
				+ "&lost_mean=" + std::to_string(lostMean) + "&lost_var=" + lostVarString
				+ "&found_mean=" + std::to_string(foundMean) + "&found_var=" + foundVarString
				+ "&lost2found_mean=" + std::to_string(lost2foundMean) + "&lost2found_var=" + lost2foundMeanString
				+ "&found2lost_mean=" + std::to_string(found2lostMean) + "&found2lost_var=" + found2lostMeanString
				+ "&kilobytes=" + kilobytes
				+ "";*/

	
			std::cout << "DATA: " << data << std::endl;

			CURL *curl;
			CURLcode result;

			/* In windows, this will init the winsock stuff */
			curl_global_init(CURL_GLOBAL_ALL);

			/* get a curl handle */
			curl = curl_easy_init();
			char *response = NULL;

			if (curl) {
				/*First set the URL that is about to receive our POST. This URL can
				just as well be a https:// URL if that is what should receive the
				data. */
				struct curl_slist *headers = NULL;
				headers = curl_slist_append(headers, "Accept: application/json");
				headers = curl_slist_append(headers, "Content-Type: application/json");
				headers = curl_slist_append(headers, "charsets: utf-8");

				
				curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/efop/handle_data.php");
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
				/* Now specify the POST data */
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

				//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_func);
				//curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
				/* Perform the request, result will get the return code */
				result = curl_easy_perform(curl);
				//printf("RESPONSE: %s\n", response);
				/* Check for errors */
				if (res != CURLE_OK)
					fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(result));
				
				/* always cleanup */
				curl_easy_cleanup(curl);
			}
			curl_global_cleanup();

			


			/*QString js = "{\"player\": {\"name\": NULL, \"gender\": NULL, \"age\": NULL, \"plays_regularly\": NULL, \"is_esport_athlete\": NULL}, \"measurement_data\": {\"bps\": "
				+ QString::number(brainBThread->get_bps()) + ", \"nop\": " + QString::number(brainBThread->get_nofPaused())
				+ ", \"lost_mean\": " + QString::number(lostMean) + ", \"lost_var\": " + QString::number(lostVar)
					+ ", \"found_mean\": " + QString::number(foundMean) + ", \"found_var\": " + QString::number(foundVar)
					+ ", \"lost2found_mean\": " + QString::number(lost2foundMean) + ", \"lost2found_var\": " + QString::number(lost2foundVar)
					+ ", \"found2lost_mean\": " + QString::number(found2lostMean) + ", \"found2lost_var\": " + QString::number(found2lostVar)
					+ ", \"kilobytes\": " + QString::number(res)
					+ "}}";

			QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);
			connect(networkManager, SIGNAL(finished(QNetworkReply*)),
				this, SLOT(serviceRequestFinished(QNetworkReply*)));

			QByteArray postData = js.toUtf8();
			QUrl serviceUrl =  QUrl(QString::fromStdString("http://localhost/efop/handle_data.php")); //TODO Ne localhost
			QNetworkRequest request(serviceUrl);
			request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
			networkManager->post(request, postData);*/
        }
        return ret;
    }


public slots :

    void updateHeroes ( const QImage &image, const int &x, const int &y );
    //void stats ( const int &t );
    void endAndStats ( const int &t );
};

#endif // BrainBWin
