using UnityEngine;
using System.Collections;

public class Movement : MonoBehaviour {

	public VisageTracker Tracker;
	
	public Transform HeadBone;
	public Transform RootBone;
	
	public Vector3 TranslationScale;
	public Vector3 TranslationOffset;
	public int TranslationFilterWindowSize;
	public float TranslationFilterAmount;
	private Vector3[] translationHistory;
	
	public Vector3 HeadToRoot;
	public Vector3 HeadRootOffset;
	
	public float RotationScale;
	public Vector3 RotationOffset;
	public int RotationFilterWindowSize;
	public float RotationFilterAmount;
	private Vector3[] rotationHistory;

	private ActionUnitData smile_left;
	private ActionUnitData smile_right;
	private ActionUnitData brow_left;
	private ActionUnitData brow_right;

	public float falseTranslationValue = 0.02f;
	public float eyebrowTreshold = 0.2f;

	public float falseRotationValue = 12f;
	private float rotationFixScale = 0f;
	
	// Use this for initialization
	void Start () 
	{
		if (Tracker == null)
			Debug.LogError("Assign a tracker", this);
		
		if (HeadBone == null)
			Debug.LogError("Assign a head bone transform", this);
		
		if (RootBone == null)
			Debug.LogError("Assign the root bone transform", this);
		
		// initialize histories
		translationHistory = new Vector3[TranslationFilterWindowSize];
		rotationHistory = new Vector3[RotationFilterWindowSize];
		
		// get head to root
		Vector3 headRoot = HeadBone.position + HeadBone.right * HeadRootOffset.x + HeadBone.up * HeadRootOffset.y + HeadBone.forward * HeadRootOffset.z;
		HeadToRoot = RootBone.position - headRoot;

	}
	
	// Update is called once per frame
	void Update () 
	{
		if (Tracker.TrackerStatus != TrackStatus.Ok)
		{
			return;
		}

		FixFalseMovement();
		
		// move head
		Vector3 translation = Tracker.Translation;
		translation = new Vector3(translation.x * TranslationScale.x, translation.y * TranslationScale.y, translation.z * TranslationScale.z);
		
		// push back translation history
		for (int i = 1; i < TranslationFilterWindowSize; i++)
			translationHistory[i - 1] = translationHistory[i];
		
		// add translation to history
		translationHistory[TranslationFilterWindowSize - 1] = translation;
		
		// filter value
		Vector3 filteredTranslation = Filter(translation, translationHistory, TranslationFilterAmount);

		HeadBone.position = filteredTranslation;
		Vector3 oldHeadPosition = HeadBone.position;
		Vector3 headRoot = HeadBone.position + HeadBone.right * HeadRootOffset.x + HeadBone.up * HeadRootOffset.y + HeadBone.forward * HeadRootOffset.z;
		RootBone.position = headRoot + HeadToRoot;
		HeadBone.position = oldHeadPosition;

		Vector3 rotation = (Tracker.Rotation + RotationOffset) * RotationScale;

		// push back rotation history
		for (int i = 1; i < RotationFilterWindowSize; i++)
			rotationHistory[i - 1] = rotationHistory[i];
		
		// add rotation to history
		rotationHistory[RotationFilterWindowSize - 1] = rotation;
		
		// filter value
		Vector3 filteredRotation = Filter(rotation, rotationHistory, RotationFilterAmount);
		HeadBone.localEulerAngles = filteredRotation;
	}
	
	private float Filter(float value, float[] history, float amount)
	{
		// get maximum variation
		float maxVariation = 0f;
		for (int i = 0; i < history.Length - 1; i++)
			maxVariation = Mathf.Max(maxVariation, Mathf.Abs(value - history[i]));
		
		// get weights
		float[] weights = new float[history.Length];
		for (int i = 0; i < weights.Length; i++)
		{
			weights[i] = Mathf.Exp(-i * amount * maxVariation);
		}
		
		// get sum of weights
		float weightSum = 0f;
		for (int i = 0; i < weights.Length; i++)
			weightSum += weights[i];
		
		// filter value
		float filteredValue = 0f;
		for (int i = 0; i < weights.Length; i++)
		{
			filteredValue += weights[i] * history[i] / weightSum;
		}
		
		return filteredValue;
	}
	
	private Vector3 Filter(Vector3 value, Vector3[] history, float amount)
	{
		// construct histories
		float[] xHistory = new float[history.Length];
		float[] yHistory = new float[history.Length];
		float[] zHistory = new float[history.Length];
		for (int i = 0; i < history.Length; i++)
		{
			xHistory[i] = history[i].x;
			yHistory[i] = history[i].y;
			zHistory[i] = history[i].z;
		}
		
		// filter values
		float x = Filter(value.x, xHistory, amount);
		float y = Filter(value.y, yHistory, amount);
		float z = Filter(value.z, zHistory, amount);
		
		return new Vector3(x, y, z);
	}

	// a quick fix for some false movements detected during tracking
	private void FixFalseMovement() {

		// fix for: false rotation while smiling

		smile_left = Tracker.GetActionUnit("au_lip_stretcher_left");
		smile_right = Tracker.GetActionUnit("au_lip_stretcher_right");

		rotationFixScale = Mathf.Clamp01((Tracker.Rotation.x + 17f) / 34f);
		if (smile_left != null && smile_left.Value > 0) {
				Tracker.Rotation.x -= falseRotationValue * smile_left.Value * rotationFixScale;
		}
		if (smile_right != null && smile_right.Value > 0) {
				Tracker.Rotation.x -= falseRotationValue * smile_right.Value * rotationFixScale;
		}

		// fix for: false translation while raising eyebrows
		brow_left = Tracker.GetActionUnit("au_left_outer_brow_raiser");
		brow_right = Tracker.GetActionUnit("au_right_outer_brow_raiser");

		Tracker.Translation.z += falseTranslationValue * Mathf.Clamp01((brow_left.Value + brow_right.Value - 0.1f) / 0.3f);
	}
}
