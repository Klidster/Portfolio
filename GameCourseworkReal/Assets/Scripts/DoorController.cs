using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class DoorController : MonoBehaviour
{

    bool isLocked = true;
    bool isOpen = false;
    public bool requireKey = true;

    [Header("Door Settings")]
    public float openAngle = 90f;              
    public float openSpeed = 2f;               

    [Header("Key Settings")]
    public bool inRange = false;
    public Inventory inventory;
    public GameObject key;
    public AudioClip unlockSfx;

    [Header("UI Prompt")]
    public Text promptText;

    private Quaternion closedRotation;
    private Quaternion openRotation;
    private Transform player;

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            inRange = true;
            if (requireKey)
                inventory.ItemUsed += OnInventoryItemUsed;

            if (promptText != null)
                promptText.gameObject.SetActive(true);
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            inRange = false;
            if (requireKey)
                inventory.ItemUsed -= OnInventoryItemUsed;

            if (promptText != null)
                promptText.gameObject.SetActive(false);

        }
    }

    private void OnInventoryItemUsed(InventoryItem item)
    {
        if ((item as MonoBehaviour).gameObject == key
            && inRange
            && isLocked)
        {
            isOpen = !isOpen;
            StopAllCoroutines();
            StartCoroutine(RotateDoor(isOpen ? openRotation : closedRotation));

            isLocked = false;
            inventory.removeItem(item);
            Debug.Log("Unlocked the door");
            AudioSource.PlayClipAtPoint(unlockSfx, Camera.main.transform.position, 0.8f);
        }
    }

    void Start()
    {
        closedRotation = transform.rotation;
        openRotation = closedRotation * Quaternion.Euler(0f, openAngle, 0f);

        if (inventory == null)
            Debug.LogError("DoorController: inventory reference not set!");

        if (promptText != null)
            promptText.gameObject.SetActive(false);

    }

    void Update()
    {
        if (inRange && Input.GetKeyDown(KeyCode.E))
        {
            if (requireKey && isLocked)
            {
                Debug.Log("The door is locked.");
                return;
            }

            isOpen = !isOpen;
            StopAllCoroutines();
            StartCoroutine(
              RotateDoor(isOpen ? openRotation : closedRotation)
            );
        }
    }

    private IEnumerator RotateDoor(Quaternion targetRot)
    {

        while (Quaternion.Angle(transform.rotation, targetRot) > 0.01f)
        {
            transform.rotation = Quaternion.Slerp(
                transform.rotation,
                targetRot,
                Time.deltaTime * openSpeed
            );
            yield return null;
        }
        transform.rotation = targetRot;
    }
}
