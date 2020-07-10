#include <zmq.h>
#include <qstring.h>
#include <assert.h>
#include <qdebug.h>
#include <QFile>

static char address_app_2 [] = "tcp://localhost:5556";
static char address_app_1 [] = "tcp://localhost:5554";

static const QString app1 = "app1_to_proxy";
static const QString app2 = "app2_to_proxy";

static int const size_of_data_from_client = 60;

static char socket_proxy [] = "tcp://*:5555";

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

    QString payload_for_log = string_buffer; // prepare pay load for log file
    payload_for_log.remove(0,app1.length());

    if (send_get_data == proxy_send_data) // if server send data to app
    {
        if (string_buffer.contains(app1))// if message for app 2
        {
            string_for_write_in_log_file = "Proxy send data to app 2 \n" + payload_for_log;
        }
        else if (string_buffer.contains(app2)) // if message for app 1
        {
            string_for_write_in_log_file = "Proxy send data to app 1 \n" + payload_for_log;
        }
    }
    else if (send_get_data == proxy_get_data) // if server get data from app
    {
        if (string_buffer.contains(app1)) // if message from app 1
        {
            string_for_write_in_log_file = "Proxy got data from app 1 \n" + payload_for_log;
        }
        else if (string_buffer.contains(app2)) // if message from app 2
        {
            string_for_write_in_log_file = "Proxy got data from app 2 \n" + payload_for_log;
        }
    }

    QByteArray ba = string_for_write_in_log_file.toLocal8Bit(); // write to file
    ba.append("\n");ba.append("\n");
    log_file.write(ba); log_file.close();
}

void SendDataToApp (QString string_buffer)
{
    // prepare data for char array
    QByteArray ba = string_buffer.toLocal8Bit();
    char buffer_for_app1 [string_buffer.length()];
    for (int i = 0; i < string_buffer.length();i++)
          buffer_for_app1[i] = ba[i];

    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);

    if (string_buffer.contains(app1))
    {
        qDebug () << "Data from server to app 2:";
        zmq_connect (requester, address_app_2);
    }
    else if (string_buffer.contains(app2))
    {
        qDebug () << "Data from server to app 1:";
        zmq_connect (requester, address_app_1);
    }

    string_buffer.remove(0,app1.length());
    qDebug () << string_buffer;
    qDebug () << "";

    zmq_send (requester, buffer_for_app1, sizeof (buffer_for_app1), 0);

    zmq_close (requester);
    zmq_ctx_destroy (context);
}

bool ProcessDataFromClient (QString string_buffer)
{
   if (string_buffer.contains(app1)) // if message from app 1
   {
       qDebug () << "Get data from app1 to server:";
       string_buffer.remove(0,app1.length());
       qDebug () << string_buffer;
       return true;
   }
   else if (string_buffer.contains(app2)) // if message from app 2
   {
       qDebug () << "Get data from app2 to server:";
       string_buffer.remove(0,app1.length());
       qDebug () << string_buffer;
       return true;
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
        char buffer [size_of_data_from_client]; memset(buffer,0,size_of_data_from_client);
        zmq_recv (responder, buffer, size_of_data_from_client, 0);

        printf ("Server:\n");

        QString string_buffer (buffer);

        bool flag_valid_data = ProcessDataFromClient (string_buffer);

        if (flag_valid_data == true)
        {
            SaveDataToLogFile (string_buffer, proxy_get_data);

            SendDataToApp (string_buffer);
            SaveDataToLogFile (string_buffer, proxy_send_data);
        }

        zmq_send (responder, "0", 1, 0); // send answer one byte

    }
    return 0;
}



