Lightweight ACK Flow

If you want to keep a simple peer-to-peer (P2P) setup, you can manually reproduce a basic reliable datagram pattern using RadioLib's non-blocking/interrupt methods. The library maintainer officially recommends avoiding blocking loops and instead structuring a Ping-Pong state machine:Keep both nodes in continuous listen mode by using radio.startReceive().

Sender Transmits: Prepend your data payload with 3 or 4 control bytes (e.g., [Target_ID] [Sender_ID] [Packet_Sequence_Num]). Send using non-blocking transmit, then switch immediately back to listen mode.

Receiver Acknowledges: Upon successfully reading a valid packet, the receiver processes it and immediately replies with a dedicated short ACK packet (e.g., containing the same sequence number) before dropping back to listen mode.

Timeout Handler: On the sender side, if a hardware timer/timeout interrupt triggers before the matching ACK packet is caught, increment an attempt counter and retransmit the packet.