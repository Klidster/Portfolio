using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(Collider))]
public class PlankCatapult : MonoBehaviour
{
    [Header("Teleport Settings")]
    public Inventory inventory;
    public InventoryItem requiredItem;
    public Transform destination;

    [Header("Screen Fade Settings")]
    public CanvasGroup blackScreenCanvas;
    public float fadeDuration = 1f;
    public float blackoutDuration = 5f;
    public AudioClip catapultSfx;

    bool _playerInZone;
    CharacterController _cc;
    Transform _playerT;

    void Awake()
    {

        var col = GetComponent<Collider>();
        col.isTrigger = true;


        if (blackScreenCanvas != null)
        {
            blackScreenCanvas.alpha = 0f;
            blackScreenCanvas.gameObject.SetActive(false);
        }
    }

    void OnEnable()
    {
        inventory.ItemUsed += OnItemUsed;
    }

    void OnDisable()
    {
        inventory.ItemUsed -= OnItemUsed;
    }

    void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            _playerInZone = true;
            _playerT = other.transform;
            _cc = other.GetComponent<CharacterController>();
        }
    }

    void OnTriggerExit(Collider other)
    {
        if (other.CompareTag("Player"))
            _playerInZone = false;
    }

    void OnItemUsed(InventoryItem item)
    {

        if (!_playerInZone || item != requiredItem)
            return;

        StartCoroutine(DoTeleportWithFade(item));
        AudioSource.PlayClipAtPoint(catapultSfx, Camera.main.transform.position, 0.9f);
    }

    IEnumerator DoTeleportWithFade(InventoryItem item)
    {

        if (blackScreenCanvas != null)
        {
            blackScreenCanvas.gameObject.SetActive(true);
            float t = 0f;
            while (t < fadeDuration)
            {
                blackScreenCanvas.alpha = Mathf.Lerp(0f, 1f, t / fadeDuration);
                t += Time.deltaTime;
                yield return null;
            }
            blackScreenCanvas.alpha = 1f;
        }
        AudioSource.PlayClipAtPoint(catapultSfx,Camera.main.transform.position, 0.9f);

        if (_cc != null)
            _cc.enabled = false;


        _playerT.position = destination.position;
        _playerT.rotation = destination.rotation;

        inventory.removeItem(item);

        yield return new WaitForSeconds(blackoutDuration);

        if (blackScreenCanvas != null)
        {
            float t = 0f;
            while (t < fadeDuration)
            {
                blackScreenCanvas.alpha = Mathf.Lerp(1f, 0f, t / fadeDuration);
                t += Time.deltaTime;
                yield return null;
            }
            blackScreenCanvas.alpha = 0f;
            blackScreenCanvas.gameObject.SetActive(false);
        }

        if (_cc != null)
            _cc.enabled = true;
    }
}
