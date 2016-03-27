using UnityEngine;
using System.Collections;

public class EyeMovement : MonoBehaviour {

	public VisageTracker Tracker;
	
	public Transform HeadBone;
	public Transform LeftEyeBone;
	public Transform RightEyeBone;
	public Transform LookAt;
	public Vector3 LookAtRotationOffset;
	public float HorizontalLookAtScale;
	public float HorizontalLookAtLimit;
	public float VerticalLookAtScale;
	public float VerticalLookAtLimit;
	
	public bool AutoBlinking;
	public SkinnedMeshRenderer BlinkMeshRenderer;
	public float BlinkInterval;
	public float BlinkIntervalDeviation;
	public float BlinkDuration;
	public int[] BlinkBlendshapeIndexes;
	private float[] blinkDesiredValue = new float[2];
	
	void Start()
	{
		// start blinking
		if (AutoBlinking)
			StartCoroutine(Blink());
	}
	
	void Update()
	{
		// look at
		if (LookAt != null && LeftEyeBone != null && RightEyeBone != null)
		{
			LeftEyeBone.LookAt(LookAt.position, HeadBone.up);
			LeftEyeBone.localEulerAngles += LookAtRotationOffset;
			RightEyeBone.LookAt(LookAt.position, HeadBone.up);
			RightEyeBone.localEulerAngles += LookAtRotationOffset;
		}
		
		// look at from tracker
		if (LookAt == null && LeftEyeBone != null && RightEyeBone != null)
		{
			float horizontal = Mathf.Clamp(Tracker.GazeDirection.x * HorizontalLookAtScale, -HorizontalLookAtLimit, HorizontalLookAtLimit);
			float vertical = Mathf.Clamp(Tracker.GazeDirection.y * VerticalLookAtScale, -VerticalLookAtLimit, VerticalLookAtLimit);
			LeftEyeBone.localRotation = Quaternion.AngleAxis(-horizontal, Vector3.up) * Quaternion.AngleAxis(vertical, Vector3.right);
			LeftEyeBone.localEulerAngles += LookAtRotationOffset;
			RightEyeBone.localRotation = Quaternion.AngleAxis(-horizontal, Vector3.up) * Quaternion.AngleAxis(vertical, Vector3.right);
			RightEyeBone.localEulerAngles += LookAtRotationOffset;
		}
		
		// update blink
		if (AutoBlinking)
		{
			float blink1 = BlinkMeshRenderer.GetBlendShapeWeight(BlinkBlendshapeIndexes[0]);
			float blink2 = BlinkMeshRenderer.GetBlendShapeWeight(BlinkBlendshapeIndexes[1]);
			blink1 = Mathf.Lerp(blink1, blinkDesiredValue[0], 0.7f);
			blink2 = Mathf.Lerp(blink2, blinkDesiredValue[1], 0.7f);
			BlinkMeshRenderer.SetBlendShapeWeight(BlinkBlendshapeIndexes[0], blink1);
			BlinkMeshRenderer.SetBlendShapeWeight(BlinkBlendshapeIndexes[1], blink2);
		}
	}
	
	private IEnumerator Blink()
	{
		blinkDesiredValue[0] = 0f;
		blinkDesiredValue[1] = 0f;
		
		yield return new WaitForSeconds(BlinkInterval + Random.Range(-BlinkIntervalDeviation, BlinkIntervalDeviation));
		
		blinkDesiredValue[0] = 100f;
		blinkDesiredValue[1] = 100f;
		
		yield return new WaitForSeconds(BlinkDuration);
		StartCoroutine(Blink());
	}
}
