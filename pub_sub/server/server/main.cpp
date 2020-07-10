#include <zmq.h>
#include <qstring.h>
#include <assert.h>
#include <qdebug.h>
#include <QFile>
#include <unistd.h>

static char address_subscriber [] = "tcp://localhost:5556";

static int const size_of_data_from_client = 60;

static char socket_proxy [] = "tcp://*:5555";

static const QStringList data_pattern_which_need_subscriber = { "222", "444"};

enum GET_SEND_DATA
{
  proxy_send_data,
  proxy_get_data
};

void SaveDataToLogFile (QString string_buffer, int send_get_data)
{
    QFile log_file ("log_proxy");
    log_file.open(QIODevice::Append);

    QString string_for_write_in_log_file {};

    if (send_get_data == proxy_get_data)
        string_for_write_in_log_file = "Proxy get data from publisher \n" + string_buffer;

    if (send_get_data == proxy_send_data)
        string_for_write_in_log_file = "Proxy send data to subscriber \n" + string_buffer;

    QByteArray ba = string_for_write_in_log_file.toLocal8Bit(); // write to file
    ba.append("\n");ba.append("\n");
    log_file.write(ba); log_file.close();
}

void SendDataToSubscriber (QString string_buffer)
{
    // prepare data for char array
    QByteArray ba = string_buffer.toLocal8Bit();
    char buffer_for_subscriber [string_buffer.length()];
    for (int i = 0; i < string_buffer.length();i++)
          buffer_for_subscriber[i] = ba[i];

    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);

    qDebug () << "Data from server to subscriber:";
    zmq_connect (requester, address_subscriber);

    qDebug () << string_buffer;
    qDebug () << "";

    zmq_send (requester, buffer_for_subscriber, sizeof (buffer_for_subscriber), 0);

    zmq_close (requester);
    zmq_ctx_destroy (context);
}

bool ProcessDataFromPublisher (QString string_buffer)
{
    if (string_buffer.length() != 0)
    {
        qDebug () << "Get data from publisher to server:";
        qDebug () << string_buffer;
    }

   for (int i = 0; i < data_pattern_which_need_subscriber.length(); i++)
   {
      if (string_buffer.contains(data_pattern_which_need_subscriber[i]))
      {
           return true; // if subscriber need data - return true
      }
   }

   return false;
}

int main (void)
{
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, socket_proxy );
    assert (rc == 0);
    printf ("Server started\n");

    while (true)
    {
        sleep (1);

        char buffer [size_of_data_from_client]; memset(buffer,0,size_of_data_from_client);
        zmq_recv (responder, buffer, size_of_data_from_client, 0);

        QString string_buffer (buffer);

        bool get_data_for_subscriber = ProcessDataFromPublisher (string_buffer);

        if (get_data_for_subscriber == false) // if data no valid for subscriber - exit
        {
           zmq_send (responder, "0", 1, 0); // send answer one byte
           continue;
        }
        else // if data valid save to log and send to subscriber
        {
            SaveDataToLogFile (string_buffer, proxy_get_data);

            SendDataToSubscriber (string_buffer);
            SaveDataToLogFile (string_buffer, proxy_send_data);
        }
    }
    return 0;
}



