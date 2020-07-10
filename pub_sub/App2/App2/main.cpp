#include <zmq.h>
#include <qstring.h>
#include <assert.h>
#include <qdebug.h>

static char socket_app_2  [] = "tcp://*:5556";

static int const size_of_data_from_client = 60;

void ProcessDataFromServer (QString string_buffer)
{
   qDebug () << "";
   qDebug () << "Get data from proxy:";
   qDebug () << string_buffer;
}

int main (void)
{
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, socket_app_2);
    assert (rc == 0);

    qDebug () << "Client subscriber started. waiting data from proxy";

    while (true)
    {
        char buffer [size_of_data_from_client]; memset(buffer,0,size_of_data_from_client);
        zmq_recv (responder, buffer, size_of_data_from_client, 0);

        QString string_buffer_proxy (buffer);

        ProcessDataFromServer (string_buffer_proxy);

        zmq_send (responder, "0", 1, 0); // send one byte answer
    }
    return 0;
}
