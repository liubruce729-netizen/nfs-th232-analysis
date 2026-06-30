
{  

 GNetClientRoot *net = new GNetClientRoot("localhost"); //
 net->SetPort (9090);
net->SendCommand("GRU RUN  STOP");
}


