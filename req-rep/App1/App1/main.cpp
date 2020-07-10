#include <zmq.h>
#include <qstring.h>
#include <qdebug.h>

static char address_proxy [] = "tcp://localhost:5555";
static char socket_app_1 [] = "tcp://*:5554";

static const QString app1 = "app1_to_proxy"; // flag. if message inculded this string it means that data from app1
static const QString app2 = "app2_to_proxy";

static int const size_of_data_from_client = 60; // max symbol which user can write via keyboard

void SendDataToProxy ()
{
    qDebug () << "";     qDebug () << "Client app 1 started";

    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, address_proxy);

    QTextStream cin (stdin);
    QString input_user_string{};

    qDebug () << "Please write data for send to proxy and press enter:";
    input_user_string = cin.readLine();

    if (input_user_string.contains(app1) || input_user_string.length() > size_of_data_from_client)
    {qDebug () << "Error. Try one more time";  return;}

    QString data_for_send_to_proxy = app1 + input_user_string; // add flat to user message

    QByteArray ba = data_for_send_to_proxy.toLocal8Bit(); // convert string to char array
    char buffer_for_send_to_proxy [data_for_send_to_proxy.length()];
    for (int i = 0; i < data_for_send_to_proxy.length();i ++)
          buffer_for_send_to_proxy[i] = ba[i];

    zmq_send (requester, buffer_for_send_to_proxy, sizeof (buffer_for_send_to_proxy), 0); // send data

    qDebug () << ""; qDebug () << "";
    qDebug () << "data_from_app1_to_proxy: "; qDebug () << input_user_string; qDebug () << "";

    zmq_close (requester);
    zmq_ctx_destroy (context);
}

int main()
{
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, socket_app_1);
    assert (rc == 0);

    while (true)
    {
        SendDataToProxy (); // send data to user via keyboard

        char buffer [size_of_data_from_client]; memset(buffer,0,size_of_data_from_client);
        zmq_recv (responder, buffer, size_of_data_from_client, 0);

        QString string_buffer_from_proxy (buffer);

        if (!string_buffer_from_proxy.contains(app2)) // get data only from app2
            break;

        qDebug  () << "App 1 get data from proxy: ";
        string_buffer_from_proxy.remove(0,app1.length()); // have only pay load
        qDebug  () << string_buffer_from_proxy;

        zmq_send (responder, "0", 1, 0); // send one byte answer
    }

    return 0;
}
