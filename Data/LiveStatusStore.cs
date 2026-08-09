

public class LiveStatusStore
{
    private LiveStatusDtos LiveStatus { get; set; } = new LiveStatusDtos();
    private readonly object lockObj = new object();

    // update the live status with new values
    public void UpdateLiveStatus(bool doorOpen, float soundValue)
    {
        //  combine into one step to avoid race conditions
        lock (lockObj)
        {
            LiveStatus.DoorOpen = doorOpen;
            LiveStatus.SoundValue = soundValue;
        }
    }

    // retrieve the current live status
    public LiveStatusDtos GetLiveStatusDtos()
    {
         lock (lockObj)
        {
            return LiveStatus;
        }
        
    }


}
