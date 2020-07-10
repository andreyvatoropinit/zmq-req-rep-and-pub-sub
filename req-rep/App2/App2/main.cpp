#include <zmq.h>
#include <qstring.h>
#include <assert.h>
#include <qdebug.h>

static char address_proxy [] = "tcp://localhost:5555";
static char socket_app_2  [] = "tcp://*:5556";

static const QString app1 = "app1_to_proxy";
static const QString app2 = "app2_to_proxy";

static int const size_of_data_from_client = 60;

void SendDataToProxy (QString string_buffer)
{
    qDebug () << "";
    qDebug () << "Client App2 started";

    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, address_proxy);

    string_buffer.remove(0,app2.length()); // have only pay load. flag deleted

    std::reverse (string_buffer.begin(),string_buffer.end()); // reverse string

    QString data_for_send_to_proxy = app2 + string_buffer; // flag app 2 and message

    QByteArray ba = data_for_send_to_proxy.toLocal8Bit(); // convert string to char array
    char buffer_for_send_to_proxy [data_for_send_to_proxy.length()];
    for (int i = 0; i < data_for_send_to_proxy.length();i ++)
          buffer_for_send_to_proxy[i] = ba[i];

    zmq_send (requester, buffer_for_send_to_proxy, sizeof (buffer_for_send_to_proxy), 0);

    qDebug () << "Data from app 2 to server sended:";
    qDebug () << string_buffer;

    zmq_close (requester);
    zmq_ctx_destroy (context);
}


bool ProcessDataFromServer (QString string_buffer)
{

   if (string_buffer.contains(app1)) // if message from app 1
   {
       qDebug () << "";
       qDebug () << "Get data from proxy:";
       string_buffer.remove(0,app1.length()); // show only pay load
       qDebug () << string_buffer;
       return true;
   }

   return false;
}

int main (void)
{
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, socket_app_2);
    assert (rc == 0);

    qDebug () << "Client App2 started. waiting data from proxy";

    while (true)
    {
        char buffer [size_of_data_from_client]; memset(buffer,0,size_of_data_from_client);
        zmq_recv (responder, buffer, size_of_data_from_client, 0);

        QString string_buffer_proxy (buffer);

        bool get_data_from_proxy = ProcessDataFromServer (string_buffer_proxy);

        if (get_data_from_proxy == true)
            SendDataToProxy (string_buffer_proxy);

        zmq_send (responder, "0", 1, 0); // send one byte answer

    }
    return 0;
}
