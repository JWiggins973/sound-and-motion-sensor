public class AlertEvent
{   
     public int Id {get; set;}
    public string Message{get; set;} = "";
    public float SoundValue{get; set;}
    public float RawPeak{get; set;}
    public DateTime Timestamp {get; set;}
}