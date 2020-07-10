#include <zmq.h>
#include <qstring.h>
#include <qdebug.h>
#include <unistd.h>

static char address_proxy [] = "tcp://localhost:5555";
static char socket_publisher [] = "tcp://*:5554";


void SendDataToProxy (QString data_for_subscriber)
{
    qDebug () << "";     qDebug () << "Publisher 1 started";

    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, address_proxy);

    QByteArray ba = data_for_subscriber.toLocal8Bit(); // convert string to char array
    char buffer_for_send_to_proxy [data_for_subscriber.length()];
    for (int i = 0; i < data_for_subscriber.length();i ++)
          buffer_for_send_to_proxy[i] = ba[i];

    zmq_send (requester, buffer_for_send_to_proxy, sizeof (buffer_for_send_to_proxy), 0); // send data

    qDebug () << ""; qDebug () << "";
    qDebug () << "data_from_publisher_to_subscriber "; qDebug () << data_for_subscriber; qDebug () << "";

    zmq_close (requester);
    zmq_ctx_destroy (context);
}


int main()
{
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, socket_publisher);
    assert (rc == 0);

    const QStringList data_from_publisher_to_subscriber = {"111", "222", "333", "444"};

    int counter = 0;

    while (true) // in cycle string by string send data to proxy
    {  
        SendDataToProxy (data_from_publisher_to_subscriber[counter]); // send data to user via keyboard

        zmq_send (responder, "0", 1, 0); // send one byte answer

        sleep(3);

        counter++;

        if (counter == data_from_publisher_to_subscriber.length()) // reload counter
            counter = 0;
    }

    return 0;
}
